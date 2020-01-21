#include "nk_backend.hpp"

#include <nova_renderer/nova_renderer.hpp>
#include <nova_renderer/procedural_mesh.hpp>
#include <nova_renderer/rhi/swapchain.hpp>

#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#include <utility>

#include <nova_renderer/loading/shaderpack_loading.hpp>
#include <nuklear.h>

#include "nova_renderer/util/logger.hpp"

#include "../../external/nova-renderer/external/glfw/include/GLFW/glfw3.h"
#include "../util/constants.hpp"
using namespace nova::renderer;
using namespace shaderpack;
using namespace rhi;

namespace nova::bf {
    const std::string FONT_PATH = BEST_FRIEND_DATA_DIR "fonts/DroidSans.ttf";

    const std::string FONT_ATLAS_NAME = "BestFriendFontAtlas";

    constexpr PixelFormatEnum UI_ATLAS_FORMAT = PixelFormatEnum::RGBA8;
    constexpr std::size_t UI_ATLAS_WIDTH = 512;
    constexpr std::size_t UI_ATLAS_HEIGHT = 512;

    struct UiDrawParams {
        glm::mat4 projection;
    };

    struct UiBatch {
        DescriptorSet* descriptors;
        MapAccessor<MeshId, ProceduralMesh> mesh;
    };

    struct NuklearTextureIdVertex : NuklearVertex {
        uint32_t texture_id;
    };

    nk_buttons to_nk_mouse_button(uint32_t button);

    NuklearImage::NuklearImage(TextureResourceAccessor image, const struct nk_image nk_image)
        : image(std::move(image)), nk_image(nk_image) {}

    NullNuklearImage::NullNuklearImage(const TextureResourceAccessor& image,
                                       const struct nk_image nk_image,
                                       const nk_draw_null_texture null_tex)
        : NuklearImage(image, nk_image), nk_null_tex(null_tex) {}

    NuklearDevice::NuklearDevice(NovaRenderer& renderer) 
        : renderer(renderer), mesh(renderer.create_procedural_mesh(MAX_VERTEX_BUFFER_SIZE, MAX_INDEX_BUFFER_SIZE)) {

        name = UI_RENDER_PASS_NAME;

        allocator = std::unique_ptr<mem::AllocatorHandle<>>(renderer.get_global_allocator()->create_suballocator());

        vertices.reserve(MAX_VERTEX_BUFFER_SIZE / sizeof(NuklearVertex));
        indices.reserve(MAX_INDEX_BUFFER_SIZE / sizeof(uint32_t));

        init_nuklear();

        create_textures();

        load_font();

        register_input_callbacks();

        save_framebuffer_size_ratio();

        clear_context();
    }

    NuklearDevice::~NuklearDevice() {
        nk_buffer_free(&nk_cmds);
        nk_clear(ctx.get());
    }

    std::shared_ptr<nk_context> NuklearDevice::get_context() const { return ctx; }

    void NuklearDevice::consume_input() {
        nk_input_begin(ctx.get());

        // TODO: Handle people typing text
        // TODO: Scrolling of some sort

        // Consume keyboard input
        for(const auto& [key, is_pressed] : keys) {
            nk_input_key(ctx.get(), key, is_pressed);
        }

        keys.clear();

        // Update NK with the current mouse position
        nk_input_motion(ctx.get(), static_cast<int>(most_recent_mouse_position.x), static_cast<int>(most_recent_mouse_position.y));

        // Consume the most recent mouse button
        if(most_recent_mouse_button) {
            nk_input_button(ctx.get(),
                            most_recent_mouse_button->first,
                            static_cast<int>(most_recent_mouse_position.x),
                            static_cast<int>(most_recent_mouse_position.y),
                            most_recent_mouse_button->second);
            most_recent_mouse_button = {};
        }

        nk_input_end(ctx.get());
    }

    std::optional<NuklearImage> NuklearDevice::create_image(const std::string& name,
                                                            const std::size_t width,
                                                            const std::size_t height,
                                                            const void* image_data) {
        const auto idx = next_image_idx;
        next_image_idx++;

        auto& resource_manager = renderer.get_resource_manager();
        auto image = resource_manager.create_texture(name, width, height, PixelFormat::Rgba8, image_data, *allocator);
        if(image) {
            textures.emplace(idx, *image);

            const struct nk_image nk_image = nk_image_id(static_cast<int>(idx));

            return std::make_optional<NuklearImage>(*image, nk_image);

        } else {
            NOVA_LOG(ERROR) << "Could not create UI image " << name;

            return std::nullopt;
        }
    }

    void NuklearDevice::clear_context() const { nk_clear(ctx.get()); }

    RenderPassCreateInfo NuklearDevice::get_create_info() {
        static auto create_info = UiRenderpass::get_create_info();

        return create_info;
    }

    void NuklearDevice::render_ui(CommandList& cmds, FrameContext& frame_ctx) {
        static const nk_draw_vertex_layout_element vertex_layout[] =
            {{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct NuklearVertex, position)},
             {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct NuklearVertex, uv)},
             {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct NuklearVertex, color)},
             {NK_VERTEX_LAYOUT_END}};

        nk_convert_config config = {};
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(NuklearVertex);
        config.vertex_alignment = NK_ALIGNOF(NuklearVertex);
        config.null = null_texture->nk_null_tex;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = NK_ANTI_ALIASING_ON;
        config.line_AA = NK_ANTI_ALIASING_ON;

        // vertex_buffer and index_buffer let Nuklear write vertex information directly
        nk_convert(ctx.get(), &nk_cmds, &nk_vertex_buffer, &nk_index_buffer, &config);

        const auto pipeline = frame_ctx.nova->get_pipeline_storage().get_pipeline(UI_PIPELINE_NAME);
        if(!pipeline) {
            NOVA_LOG(ERROR) << "Could not get pipeline " << UI_PIPELINE_NAME << " from Nova's pipeline storage";
            return;
        }

        if(texture_array_descriptors.empty()) {
            create_descriptor_sets(*pipeline);
        }

        cmds.bind_pipeline(pipeline->pipeline);

        const auto& [vertex_buffer, index_buffer] = mesh->get_buffers_for_frame(frame_ctx.frame_count % NUM_IN_FLIGHT_FRAMES);
        cmds.bind_vertex_buffers({4, vertex_buffer});
        cmds.bind_index_buffer(index_buffer);

        // Textures to bind to the current descriptor set
        std::pmr::vector<Image*> current_descriptor_textures(*frame_ctx.allocator);
        current_descriptor_textures.reserve(MAX_NUM_TEXTURES);

        // Iterator to the descriptor set to write the current textures to
        auto descriptor_set_itr = texture_array_descriptors.begin();

        uint32_t num_sets_used = 0;
        uint32_t offset = 0;

        for(const nk_draw_command* cmd = nk__draw_begin(ctx.get(), &nk_cmds); cmd != nullptr;
            cmd = nk__draw_next(cmd, &nk_cmds, ctx.get())) {
            if(cmd->elem_count == 0) {
                continue;
            }

            const int tex_index = cmd->texture.id;
            const auto tex_itr = textures.find(tex_index);
            if(current_descriptor_textures.size() < MAX_NUM_TEXTURES) {
                current_descriptor_textures.emplace_back(tex_itr->second->image);

            } else {
                std::pmr::vector<DescriptorSetWrite> writes(1, {}, *frame_ctx.allocator);
                DescriptorSetWrite& write = writes[0];
                write.set = *descriptor_set_itr;
                write.binding = 0;
                write.type = DescriptorType::CombinedImageSampler;
                write.resources.reserve(current_descriptor_textures.size());

                std::transform(current_descriptor_textures.begin(),
                               current_descriptor_textures.end(),
                               std::back_insert_iterator<std::pmr::vector<DescriptorResourceInfo>>(write.resources),
                               [&](Image* image) {
                                   DescriptorResourceInfo info = {};
                                   info.image_info.image = image;
                                   info.image_info.format.pixel_format = UI_ATLAS_FORMAT;
                                   info.image_info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
                                   info.image_info.format.width = UI_ATLAS_WIDTH;
                                   info.image_info.format.height = UI_ATLAS_HEIGHT;
                                   return info;
                               });

                renderer.get_engine().update_descriptor_sets(writes);

                num_sets_used++;
                ++descriptor_set_itr;
            }

            const auto scissor_rect_x = static_cast<uint32_t>(std::max(0.0f, std::round(cmd->clip_rect.x * framebuffer_size_ratio.x)));
            const auto scissor_rect_y = static_cast<uint32_t>(std::max(0.0f, std::round(cmd->clip_rect.y * framebuffer_size_ratio.y)));
            const auto scissor_rect_width = static_cast<uint32_t>(std::max(0.0f, std::round(cmd->clip_rect.w * framebuffer_size_ratio.x)));
            const auto scissor_rect_height = static_cast<uint32_t>(std::max(0.0f, std::round(cmd->clip_rect.h * framebuffer_size_ratio.y)));
            cmds.set_scissor_rect(scissor_rect_x, scissor_rect_y, scissor_rect_width, scissor_rect_height);

            cmds.draw_indexed_mesh(cmd->elem_count, offset);

            offset += cmd->elem_count;
        }

        if(num_sets_used > 1) {
            NOVA_LOG(INFO) << "Used " << num_sets_used << " descriptor sets. Maybe you should only use one";
        }

        clear_context();
    }

    void NuklearDevice::init_nuklear() {
        ctx = std::make_shared<nk_context>();
        nk_init_default(ctx.get(), nullptr);

        nk_buffer_init_default(&nk_cmds);

        nk_buffer_init_fixed(&nk_vertex_buffer, vertices.data(), MAX_VERTEX_BUFFER_SIZE);
        nk_buffer_init_fixed(&nk_index_buffer, indices.data(), MAX_INDEX_BUFFER_SIZE);
    }

    void NuklearDevice::create_textures() {
        // Create the null texture
        const std::optional<NuklearImage> null_image = create_image(NULL_TEXTURE_NAME, 8, 8, nullptr);
        if(null_image) {
            null_texture = std::make_unique<NullNuklearImage>(null_image->image, null_image->nk_image);

        } else {
            NOVA_LOG(ERROR) << "Could not create null texture";
        }
    }

    void NuklearDevice::load_font() {
        nk_atlas = std::make_unique<nk_font_atlas>();
        nk_font_atlas_init_default(nk_atlas.get());
        nk_font_atlas_begin(nk_atlas.get());

        // Todo: load a font
        font = nk_font_atlas_add_from_file(nk_atlas.get(), FONT_PATH.c_str(), 14, nullptr);
        if(!font) {
            NOVA_LOG(ERROR) << "Could not load font " << FONT_PATH;
        }

        retrieve_font_atlas();

        nk_font_atlas_end(nk_atlas.get(), nk_handle_id(static_cast<int>(ImageId::FontAtlas)), &null_texture->nk_null_tex);
        nk_style_set_font(ctx.get(), &font->handle);
    }

    void NuklearDevice::retrieve_font_atlas() {
        int width, height;
        const void* image_data = nk_font_atlas_bake(nk_atlas.get(), &width, &height, NK_FONT_ATLAS_RGBA32);

        const auto new_font_atlas = create_image(FONT_ATLAS_NAME, width, height, image_data);
        if(new_font_atlas) {
            font_image = std::make_unique<NuklearImage>(new_font_atlas->image, new_font_atlas->nk_image);

        } else {
            NOVA_LOG(ERROR) << "Could not create font atlas texture";
        }
    }

    void NuklearDevice::register_input_callbacks() {
        auto& window = renderer.get_window();
        window.register_key_callback([&](const auto& key, const bool is_press, const bool is_control_down, const bool /* is_shift_down */) {
            std::lock_guard l(key_buffer_mutex);
            // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
            switch(key) {
                case GLFW_KEY_DELETE:
                    keys.emplace_back(NK_KEY_DEL, is_press);
                    break;

                case GLFW_KEY_ENTER:
                    keys.emplace_back(NK_KEY_ENTER, is_press);
                    break;

                case GLFW_KEY_TAB:
                    keys.emplace_back(NK_KEY_TAB, is_press);
                    break;

                case GLFW_KEY_BACKSPACE:
                    keys.emplace_back(NK_KEY_BACKSPACE, is_press);
                    break;

                case GLFW_KEY_UP:
                    keys.emplace_back(NK_KEY_UP, is_press);
                    break;

                case GLFW_KEY_DOWN:
                    keys.emplace_back(NK_KEY_DOWN, is_press);
                    break;

                case GLFW_KEY_HOME:
                    keys.emplace_back(NK_KEY_TEXT_START, is_press);
                    keys.emplace_back(NK_KEY_SCROLL_START, is_press);
                    break;

                case GLFW_KEY_END:
                    keys.emplace_back(NK_KEY_TEXT_END, is_press);
                    keys.emplace_back(NK_KEY_SCROLL_END, is_press);
                    break;

                case GLFW_KEY_PAGE_DOWN:
                    keys.emplace_back(NK_KEY_SCROLL_DOWN, is_press);
                    break;

                case GLFW_KEY_PAGE_UP:
                    keys.emplace_back(NK_KEY_SCROLL_UP, is_press);
                    break;

                case GLFW_KEY_LEFT_SHIFT:
                    [[fallthrough]];
                case GLFW_KEY_RIGHT_SHIFT:
                    keys.emplace_back(NK_KEY_SHIFT, is_press);
                    break;
            }

            if(is_control_down) {
                // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
                switch(key) {
                    case GLFW_KEY_C:
                        keys.emplace_back(NK_KEY_COPY, is_press);
                        break;

                    case GLFW_KEY_V:
                        keys.emplace_back(NK_KEY_PASTE, is_press);
                        break;

                    case GLFW_KEY_X:
                        keys.emplace_back(NK_KEY_CUT, is_press);
                        break;

                    case GLFW_KEY_Z:
                        keys.emplace_back(NK_KEY_TEXT_UNDO, is_press);
                        break;

                    case GLFW_KEY_R:
                        keys.emplace_back(NK_KEY_TEXT_REDO, is_press);
                        break;

                    case GLFW_KEY_LEFT:
                        keys.emplace_back(NK_KEY_TEXT_WORD_LEFT, is_press);
                        break;

                    case GLFW_KEY_RIGHT:
                        keys.emplace_back(NK_KEY_TEXT_WORD_RIGHT, is_press);
                        break;

                    case GLFW_KEY_B:
                        keys.emplace_back(NK_KEY_TEXT_LINE_START, is_press);
                        break;

                    case GLFW_KEY_E:
                        keys.emplace_back(NK_KEY_TEXT_LINE_END, is_press);
                        break;
                }
            } else {
                // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
                switch(key) {
                    case GLFW_KEY_LEFT:
                        keys.emplace_back(NK_KEY_LEFT, is_press);
                        break;

                    case GLFW_KEY_RIGHT:
                        keys.emplace_back(NK_KEY_RIGHT, is_press);
                        break;
                }
            }
        });

        window.register_mouse_callback([&](const double x_position, const double y_position) {
            most_recent_mouse_position = {x_position, y_position};
        });

        window.register_mouse_button_callback([&](const uint32_t button, const bool is_press) {
            most_recent_mouse_button = std::make_optional<std::pair<nk_buttons, bool>>(to_nk_mouse_button(button), is_press);
        });
    }

    void NuklearDevice::save_framebuffer_size_ratio() { framebuffer_size_ratio = renderer.get_window().get_framebuffer_to_window_ratio(); }

    void NuklearDevice::create_descriptor_sets(const renderer::Pipeline& pipeline) {
        auto& device = renderer.get_engine();
        if(pool == nullptr) {
            const auto num_sampled_images = pipeline.pipeline_interface->get_num_sampled_images();
            const auto num_samplers = pipeline.pipeline_interface->get_num_samplers();
            const auto num_uniform_buffers = pipeline.pipeline_interface->get_num_uniform_buffers();
            pool = device.create_descriptor_pool(num_sampled_images, num_samplers, num_uniform_buffers, *allocator);

        } else {
            device.reset_descriptor_pool(pool);
        }

        material_descriptors = device.create_descriptor_sets(pipeline.pipeline_interface, pool, *allocator);
    }

    nk_buttons to_nk_mouse_button(const uint32_t button) {
        switch(button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                return NK_BUTTON_LEFT;

            case GLFW_MOUSE_BUTTON_MIDDLE:
                return NK_BUTTON_MIDDLE;

            case GLFW_MOUSE_BUTTON_RIGHT:
                return NK_BUTTON_RIGHT;

            default:
                return NK_BUTTON_LEFT;
        }
    }

} // namespace nova::bf
