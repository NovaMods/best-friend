#include "nk_backend.hpp"

#include <nova_renderer/frontend/procedural_mesh.hpp>
#include <nova_renderer/nova_renderer.hpp>
#include <nova_renderer/rhi/swapchain.hpp>

#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
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

    constexpr shaderpack::PixelFormatEnum UI_ATLAS_FORMAT = PixelFormatEnum::RGBA8;
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

    NuklearDevice::NuklearDevice(NovaRenderer& renderer)
        : UiRenderpass(renderer.get_engine(), renderer.get_engine()->get_swapchain()->get_size()),
          renderer(renderer),
          mesh(renderer.create_procedural_mesh(MAX_VERTEX_BUFFER_SIZE, MAX_INDEX_BUFFER_SIZE)) {

        // TODO: Some way to validate that this pass exists in the loaded shaderpack
        const FullMaterialPassName& ui_full_material_pass_name = {"BestFriendGUI", "BestFriendGUI"};
        StaticMeshRenderableData ui_renderable_data = {};
        ui_renderable_data.mesh = mesh.get_key();
        ui_renderable_id = renderer.add_renderable_for_material(ui_full_material_pass_name, ui_renderable_data);

        vertices.reserve(MAX_VERTEX_BUFFER_SIZE / sizeof(NuklearVertex));
        indices.reserve(MAX_INDEX_BUFFER_SIZE / sizeof(uint32_t));

        init_nuklear();

        create_textures();

        create_pipeline();

        load_font();

        register_input_callbacks();
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

    NuklearImage NuklearDevice::create_image(const std::string& name, const std::size_t width, const std::size_t height) {
        const auto idx = next_image_idx;
        next_image_idx++;

        TextureCreateInfo create_info = {};
        create_info.name = name;
        create_info.usage = ImageUsage::SampledImage;
        create_info.format.pixel_format = PixelFormatEnum::RGBA8;
        create_info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
        create_info.format.width = static_cast<float>(width);
        create_info.format.height = static_cast<float>(height);

        Image* image = renderer.get_engine()->create_image(create_info);
        textures.emplace(next_image_idx, image);

        const struct nk_image nk_image = nk_image_id(static_cast<int>(next_image_idx));

        return {image, nk_image};
    }

    void NuklearDevice::render_ui(CommandList* cmds, FrameContext& frame_ctx) {
        static const nk_draw_vertex_layout_element vertex_layout[] =
            {{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct NuklearVertex, position)},
             {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct NuklearVertex, uv)},
             {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct NuklearVertex, color)},
             {NK_VERTEX_LAYOUT_END}};

        nk_convert_config config = {};
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(NuklearVertex);
        config.vertex_alignment = NK_ALIGNOF(NuklearVertex);
        config.null = null_texture;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = NK_ANTI_ALIASING_ON;
        config.line_AA = NK_ANTI_ALIASING_ON;

        // vertex_buffer and index_buffer let Nuklear write vertex information directly
        nk_convert(ctx.get(), &nk_cmds, &vertex_buffer, &index_buffer, &config);

        // For each command, add its texture to a texture array and add its mesh data to a mesh
        // When the texture array is full, allocate a new one from the RHI and begin using that for draws
        // When the mesh is full, allocate a new one from the RHI
        // We need to keep a lit of previously-used descriptor
        // The descriptors need to be CPU descriptors that we can update whenever we feel like. The RHI should create GPU descriptors on
        // draw HOWEVER, we should keep a buffer of meshes that we already used so we don't constantly reallocate meshes

        // Textures to bind to the current descriptor set
        std::vector<Image*> current_descriptor_textures;
        current_descriptor_textures.reserve(MAX_NUM_TEXTURES);

        // Iterator to the descriptor set to write the current textures to
        auto descriptor_set_itr = sets.begin();

        uint32_t num_sets_used = 0;

        for(const nk_draw_command* cmd = nk__draw_begin(ctx.get(), &nk_cmds); cmd != nullptr;
            cmd = nk__draw_next(cmd, &nk_cmds, ctx.get())) {
            if(cmd->elem_count == 0) {
                continue;
            }

            // TODO: Record MultiDrawIndirect commands for them, just for fun
            // Maybe save that in a secondary command list and avoid re-recording?

            const int tex_index = cmd->texture.id;
            const auto tex_itr = textures.find(tex_index);
            if(current_descriptor_textures.size() < MAX_NUM_TEXTURES) {
                current_descriptor_textures.emplace_back(tex_itr->second);

            } else {
                std::vector<DescriptorSetWrite> writes(1);
                DescriptorSetWrite& write = writes[0];
                write.set = *descriptor_set_itr;
                write.first_binding = 0;
                write.type = DescriptorType::CombinedImageSampler;
                write.resources.reserve(current_descriptor_textures.size());

                std::transform(current_descriptor_textures.begin(),
                               current_descriptor_textures.end(),
                               std::back_insert_iterator<std::vector<DescriptorResourceInfo>>(write.resources),
                               [&](Image* image) {
                                   DescriptorResourceInfo info = {};
                                   info.image_info.image = image;
                                   info.image_info.format.pixel_format = UI_ATLAS_FORMAT;
                                   info.image_info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
                                   info.image_info.format.width = UI_ATLAS_WIDTH;
                                   info.image_info.format.height = UI_ATLAS_HEIGHT;
                                   return info;
                               });

                renderer.get_engine()->update_descriptor_sets(writes);

                num_sets_used++;
                ++descriptor_set_itr;
            }
        }

        if(num_sets_used > 1) {
            NOVA_LOG(INFO) << "Used " << num_sets_used << " descriptor sets. Maybe you should only use one";
        }

        nk_clear(ctx.get());
    }

    void NuklearDevice::init_nuklear() {
        ctx = std::make_shared<nk_context>();
        nk_init_default(&(*ctx), nullptr);

        nk_buffer_init_default(&nk_cmds);

        nk_buffer_init_fixed(&vertex_buffer, vertices.data(), MAX_VERTEX_BUFFER_SIZE);
        nk_buffer_init_fixed(&index_buffer, indices.data(), MAX_INDEX_BUFFER_SIZE);
    }

    void NuklearDevice::create_textures() {
        // Create the null texture
    }

    void NuklearDevice::create_pipeline() {
        auto* device = renderer.get_engine();

        ResourceBindingDescription ui_tex_desc = {};
        ui_tex_desc.set = 0;
        ui_tex_desc.binding = 0;
        ui_tex_desc.count = device->info.max_unbounded_array_size;
        ui_tex_desc.is_unbounded = true;
        ui_tex_desc.type = DescriptorType::CombinedImageSampler;
        ui_tex_desc.stages = ShaderStageFlags::Fragment;

        std::unordered_map<std::string, ResourceBindingDescription> bindings;
        bindings.emplace("ui_textures", ui_tex_desc);

        TextureAttachmentInfo color_rtv_info = {};
        color_rtv_info.name = BACKBUFFER_NAME;
        color_rtv_info.pixel_format = PixelFormatEnum::RGBA8;
        color_rtv_info.clear = false;

        device->create_pipeline_interface(bindings, {color_rtv_info}, {})
            .map([&](PipelineInterface* pipeline_interface) {
                this->pipeline_interface = pipeline_interface;
                PipelineCreateInfo pipe_info = {};
                pipe_info.name = UI_PIPELINE_NAME;
                pipe_info.pass = UI_RENDER_PASS_NAME;
                pipe_info.states = {StateEnum::DisableDepthTest,
                                    StateEnum::DisableDepthWrite,
                                    StateEnum::Blending,
                                    StateEnum::StencilWrite,
                                    StateEnum::EnableStencilTest};
                pipe_info.vertex_fields = {{"POSITION", VertexFieldEnum::Position},
                                           {"TEXCOORD", VertexFieldEnum::UV0},
                                           {"COLOR", VertexFieldEnum::Color},
                                           {"INDEX", VertexFieldEnum::McEntityId}};
                // TODO: fill in the rest of the pipeline info

                device->create_pipeline(pipeline_interface, pipe_info)
                    .map([&](rhi::Pipeline* pipe) {
                        this->pipeline = pipe;
                        return true;
                    })
                    .on_error([](const ntl::NovaError& err) { NOVA_LOG(ERROR) << "Could not create UI pipeline: " << err.to_string(); });

                return true;
            })
            .on_error([](const ntl::NovaError& err) { NOVA_LOG(ERROR) << "Could not create UI pipeline interface: " << err.to_string(); });
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

        nk_font_atlas_end(nk_atlas.get(), nk_handle_id(static_cast<int>(ImageId::FontAtlas)), &null_texture);
        nk_style_set_font(ctx.get(), &font->handle);
    }

    void NuklearDevice::retrieve_font_atlas() {
        int width, height;
        const void* image_data = nk_font_atlas_bake(nk_atlas.get(), &width, &height, NK_FONT_ATLAS_RGBA32);

        font_image = create_image(FONT_ATLAS_NAME, width, height);
    }

    void NuklearDevice::register_input_callbacks() {
        const auto window = renderer.get_window();
        window->register_key_callback(
            [&](const auto& key, const bool is_press, const bool is_control_down, const bool /* is_shift_down */) {
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

        window->register_mouse_callback([&](const double x_position, const double y_position) {
            most_recent_mouse_position = {x_position, y_position};
        });

        window->register_mouse_button_callback([&](const uint32_t button, const bool is_press) {
            most_recent_mouse_button = std::make_optional<std::pair<nk_buttons, bool>>(to_nk_mouse_button(button), is_press);
        });
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
