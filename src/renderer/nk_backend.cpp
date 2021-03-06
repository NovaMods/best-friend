#include "nk_backend.hpp"

#include <nova_renderer/loading/renderpack_loading.hpp>
#include <nova_renderer/nova_renderer.hpp>
#include <nova_renderer/procedural_mesh.hpp>
#include <nova_renderer/rhi/swapchain.hpp>
#include <rx/core/log.h>

#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#include <nuklear.h>

#include "../../external/nova-renderer/external/glfw/include/GLFW/glfw3.h"
#include "../util/constants.hpp"
#include "rx/core/algorithm/max.h"

using namespace nova::renderer;
using namespace renderpack;
using namespace rhi;

namespace nova::bf {
    RX_LOG("NuklearRenderPass", logger);

    constexpr const char* FONT_PATH = BEST_FRIEND_DATA_DIR "/fonts/DroidSans.ttf";

    constexpr const char* FONT_ATLAS_NAME = "BestFriendFontAtlas";

    constexpr const char* UI_UBO_NAME = "BestFriendUiUbo";

    constexpr const char* UI_CAMERA_NAME = "UI Camera";

    struct UiDrawParams {
        glm::mat4 projection;
    };

    struct UiBatch {
        RhiDescriptorSet* descriptors;
        MapAccessor<MeshId, ProceduralMesh> mesh;
    };

    struct NuklearTextureIdVertex : NuklearVertex {
        uint32_t texture_id;
    };

    nk_buttons to_nk_mouse_button(uint32_t button);

    NuklearImage::NuklearImage(TextureResourceAccessor image_in, const struct nk_image nk_image_in)
        : image(rx::utility::move(image_in)), nk_image(nk_image_in) {}

    DefaultNuklearImage::DefaultNuklearImage(const TextureResourceAccessor& image_in,
                                             const struct nk_image nk_image_in,
                                             const nk_draw_null_texture null_tex)
        : NuklearImage(image_in, nk_image_in), nk_null_tex(null_tex) {}

    NuklearUiPass::NuklearUiPass(NovaRenderer* renderer_in)
        : nk_ctx{rx::make_ptr<nk_context>(renderer_in->get_global_allocator())},
          renderer(*renderer_in),
          mesh(renderer_in->create_procedural_mesh(MAX_VERTEX_BUFFER_SIZE, MAX_INDEX_BUFFER_SIZE)),
          allocator{renderer_in->get_global_allocator()},
          materials{allocator} {

        name = UI_RENDER_PASS_NAME;

        raw_vertices.resize(MAX_VERTEX_BUFFER_SIZE / sizeof(NuklearVertex));
        indices.resize(MAX_INDEX_BUFFER_SIZE / sizeof(uint32_t));

        init_nuklear();

        create_resources();

        load_font();

        register_input_callbacks();

        save_framebuffer_size_ratio();

        clear_context();

        create_ui_camera();
    }

    NuklearUiPass::~NuklearUiPass() {
        nk_buffer_free(&nk_cmds);
        nk_clear(nk_ctx.get());

        allocator.destroy<NuklearImage>(font_image);
        allocator.destroy<DefaultNuklearImage>(default_texture);
        allocator.destroy<nk_font_atlas>(nk_atlas);
    }

    nk_context* NuklearUiPass::get_context() const { return nk_ctx.get(); }

    void NuklearUiPass::consume_input() {
        nk_input_begin(nk_ctx.get());

        // TODO: Handle people typing text
        // TODO: Scrolling of some sort

        // Consume keyboard input
        {
            rx::concurrency::scope_lock l(key_buffer_mutex);
            keys.each_fwd([&](const std::pair<nk_keys, bool>& pair) { nk_input_key(nk_ctx.get(), pair.first, pair.second); });
            keys.clear();
        }

        {
            rx::concurrency::scope_lock l(mouse_button_buffer_mutex);
            // Update NK with the current mouse position
            nk_input_motion(nk_ctx.get(), static_cast<int>(most_recent_mouse_position.x), static_cast<int>(most_recent_mouse_position.y));

            // Consume the most recent mouse button
            mouse_buttons.each_fwd([&](const std::pair<nk_buttons, bool>& mouse_event) {
                nk_input_button(nk_ctx.get(),
                                mouse_event.first,
                                static_cast<int>(most_recent_mouse_position.x),
                                static_cast<int>(most_recent_mouse_position.y),
                                mouse_event.second);
            });
            mouse_buttons.clear();
        }

        nk_input_end(nk_ctx.get());
    }

    rx::optional<NuklearImage> NuklearUiPass::create_image(const rx::string& name,
                                                           const rx_size width,
                                                           const rx_size height,
                                                           const void* image_data) {
        auto& resource_manager = renderer.get_resource_manager();
        auto image = resource_manager.create_texture(name, width, height, PixelFormat::Rgba8, image_data, allocator);
        if(image) {
            const auto texture_id = resource_manager.get_texture_idx_for_name(name);
            logger->verbose("Texture %s was created with ID %u", name, *texture_id);

            const struct nk_image nk_image = nk_image_id(static_cast<int>(*texture_id));

            return NuklearImage{*image, nk_image};

        } else {
            logger->error("Could not create UI image %s", name);

            return rx::nullopt;
        }
    }

    void NuklearUiPass::clear_context() const { nk_clear(nk_ctx.get()); }

    void NuklearUiPass::create_ui_camera() {
        CameraCreateInfo info = {};
        info.name = UI_CAMERA_NAME;
        info.field_of_view = 0;
        ui_camera = renderer.create_camera(info);

        logger->verbose("UI camera has index %u", ui_camera->index);
    }

    const RenderPassCreateInfo& NuklearUiPass::get_create_info() { return UiRenderpass::get_create_info(); }

    void NuklearUiPass::init_nuklear() { nk_init_default(nk_ctx.get(), nullptr); }

    void NuklearUiPass::create_resources() {
        create_default_texture();

        create_ui_ubo();
    }

    void NuklearUiPass::create_default_texture() {
        rx::vector<uint8_t> default_img_data{allocator, 8 * 8 * 4};
        default_img_data.each_fwd([](uint8_t& elem) { elem = 0xFF; }); // Set the whole texture to white

        const rx::optional<NuklearImage> default_image = create_image(DEFAULT_TEXTURE_NAME, 8, 8, default_img_data.data());
        if(default_image) {
            default_texture = allocator.create<DefaultNuklearImage>(default_image->image, default_image->nk_image);

        } else {
            logger->error("Could not create null texture");
        }
    }

    void NuklearUiPass::create_ui_ubo() {
        auto& resource_manager = renderer.get_resource_manager();
        if(auto buffer = resource_manager.create_uniform_buffer(UI_UBO_NAME, sizeof(glm::mat4)); buffer) {
            ui_draw_params = rx::utility::move((*buffer)->buffer);

        } else {
            logger->error("Could not create UI UBO");
        }
    }

    void NuklearUiPass::load_font() {
        nk_atlas = allocator.create<nk_font_atlas>();
        nk_font_atlas_init_default(nk_atlas);
        nk_font_atlas_begin(nk_atlas);

        font = nk_font_atlas_add_from_file(nk_atlas, FONT_PATH, 14, nullptr);
        if(!font) {
            logger->error("Could not load font %s", FONT_PATH);
            return;
        }

        retrieve_font_atlas();

        const auto& resource_manager = renderer.get_resource_manager();
        const auto font_texture_id = resource_manager.get_texture_idx_for_name(FONT_ATLAS_NAME);

        nk_font_atlas_end(nk_atlas, nk_handle_id(static_cast<int>(*font_texture_id)), &default_texture->nk_null_tex);
        nk_style_set_font(nk_ctx.get(), &font->handle);

        logger->verbose("Loaded font %s", FONT_PATH);
    }

    void NuklearUiPass::retrieve_font_atlas() {
        int width, height;
        const void* image_data = nk_font_atlas_bake(nk_atlas, &width, &height, NK_FONT_ATLAS_RGBA32);

        const auto new_font_atlas = create_image(FONT_ATLAS_NAME, width, height, image_data);
        if(new_font_atlas) {
            font_image = allocator.create<NuklearImage>(new_font_atlas->image, new_font_atlas->nk_image);

        } else {
            logger->error("Could not create font atlas texture");
        }
    }

    void NuklearUiPass::register_input_callbacks() {
        auto& window = renderer.get_window();
        window.register_key_callback([&](const auto& key, const bool is_press, const bool is_control_down, const bool /* is_shift_down */) {
            rx::concurrency::scope_lock l(key_buffer_mutex);
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
            rx::concurrency::scope_lock l(mouse_button_buffer_mutex);
            mouse_buttons.emplace_back(to_nk_mouse_button(button), is_press);
        });
    }

    void NuklearUiPass::save_framebuffer_size_ratio() { framebuffer_size_ratio = renderer.get_window().get_framebuffer_to_window_ratio(); }

    rx::string to_string(const nk_flags flags) {
        // This performs all the allocations :(
        // Real string builder when
        rx::string str;

        if((flags & NK_CONVERT_INVALID_PARAM) != 0) {
            return "NK_CONVERT_INVALID_PARAM";
        }

        if((flags & NK_CONVERT_COMMAND_BUFFER_FULL) != 0) {
            str.append("NK_CONVERT_COMMAND_BUFFER_FULL");
        }

        if((flags & NK_CONVERT_VERTEX_BUFFER_FULL) != 0) {
            if(!str.is_empty()) {
                str.append(" | ");
            }
            str.append("NK_CONVERT_VERTEX_BUFFER_FULL");
        }

        if((flags & NK_CONVERT_ELEMENT_BUFFER_FULL) != 0) {
            if(!str.is_empty()) {
                str.append(" | ");
            }
            str.append("NK_CONVERT_ELEMENT_BUFFER_FULL");
        }

        return str;
    }

    rx::string to_string(const nk_buffer& buffer) {
        return rx::string::format("{grow_factor=%f allocated=%u needed=%u size=%u}",
                                  buffer.grow_factor,
                                  buffer.allocated,
                                  buffer.needed,
                                  buffer.size);
    }

    void NuklearUiPass::setup_renderpass(RhiRenderCommandList& cmds, FrameContext& frame_ctx) {
        static const nk_draw_vertex_layout_element VERTEX_LAYOUT[] =
            {{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct RawNuklearVertex, position)},
             {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct RawNuklearVertex, uv)},
             {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct RawNuklearVertex, color)},
             {NK_VERTEX_LAYOUT_END}};

        const auto frame_idx = static_cast<uint8_t>(frame_ctx.frame_idx);

        nk_buffer_init_default(&nk_cmds);
        nk_buffer_init_fixed(&nk_vertex_buffer, raw_vertices.data(), MAX_VERTEX_BUFFER_SIZE);
        nk_buffer_init_fixed(&nk_index_buffer, indices.data(), MAX_INDEX_BUFFER_SIZE);

        nk_convert_config config = {};
        config.vertex_layout = VERTEX_LAYOUT;
        config.vertex_size = sizeof(RawNuklearVertex);
        config.vertex_alignment = NK_ALIGNOF(RawNuklearVertex);
        config.null = default_texture->nk_null_tex;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = NK_ANTI_ALIASING_ON;
        config.line_AA = NK_ANTI_ALIASING_ON;

        // vertex_buffer and index_buffer let Nuklear write vertex information directly
        const auto result = nk_convert(nk_ctx.get(), &nk_cmds, &nk_vertex_buffer, &nk_index_buffer, &config);
        if(result != NK_CONVERT_SUCCESS) {
            logger->error("Could not convert Nuklear UI to mesh data: %s", to_string(result));
        }

        // Record the commands to upload the mesh data now, so they're before the renderpass commands in the command list
        // However, we don't put data into the staging buffers until the `render_ui` method - but that method executes before the GPU
        // executes the command list, so there's no out-of-order issues
        mesh->record_commands_to_upload_data(&cmds, frame_idx);

        const auto window_size = frame_ctx.nova->get_window().get_window_size();

        glm::mat4 ui_matrix{
            {2.0f, 0.0f, 0.0f, -1.0f},
            {0.0f, 2.0f, 0.0f, -1.0f},
            {0.0f, 0.0f, -1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        };
        ui_matrix[0][0] /= window_size.x;
        ui_matrix[1][1] /= window_size.y;

        frame_ctx.nova->get_device().write_data_to_buffer(&ui_matrix[0][0], sizeof(glm::mat4), *ui_draw_params);
    }

    void NuklearUiPass::render_ui(RhiRenderCommandList& cmds, FrameContext& frame_ctx) {
        // Pipeline state should have been loaded from the renderpack
        const auto pipeline = frame_ctx.nova->find_pipeline(UI_PIPELINE_NAME);
        if(!pipeline) {
            logger->error("Could not get pipeline %s from Nova's pipeline storage", UI_PIPELINE_NAME);
            return;
        }

        cmds.set_camera(*ui_camera);

        cmds.set_pipeline(*pipeline->pipeline);

        const auto& [vertex_buffer, index_buffer] = mesh->get_buffers_for_frame(frame_ctx.frame_idx);
        rx::vector<RhiBuffer*> vertex_buffers;
        for(uint32_t i = 0; i < 4; i++) {
            vertex_buffers.push_back(vertex_buffer);
        }
        cmds.bind_vertex_buffers(vertex_buffers);
        cmds.bind_index_buffer(*index_buffer, IndexType::Uint16);

        // Textures to bind to the current descriptor set
        rx::vector<RhiImage*> current_descriptor_textures{*frame_ctx.allocator};
        current_descriptor_textures.reserve(MAX_NUM_TEXTURES);
        current_descriptor_textures.push_back(default_texture->image->image.get());

        uint32_t num_sets_used = 0;
        uint32_t offset = 0;

        rx::vector<NuklearVertex> vertices{*frame_ctx.allocator, raw_vertices.size()};
        rx::vector<rx::pair<uint32_t, NuklearUiMaterial*>> used_materials{*frame_ctx.allocator};

        // Record drawcalls
        const nk_draw_command* cmd;
        nk_draw_foreach(cmd, nk_ctx.get(), &nk_cmds) {
            if(cmd->elem_count == 0) {
                continue;
            }

            const int tex_index = cmd->texture.id;

            auto [idx, material] = get_next_material();

            material->texture = tex_index;
            cmds.set_material_index(idx);

            // Save the material we just used so we can free it and use it next frame
            used_materials.push_back({idx, material});

            const auto scissor_rect_x = static_cast<uint32_t>(rx::algorithm::max(0.0F, round(cmd->clip_rect.x * framebuffer_size_ratio.x)));
            const auto scissor_rect_y = static_cast<uint32_t>(rx::algorithm::max(0.0F, round(cmd->clip_rect.y * framebuffer_size_ratio.y)));
            const auto scissor_rect_width = static_cast<uint32_t>(
                rx::algorithm::max(0.0F, round(cmd->clip_rect.w * framebuffer_size_ratio.x)));
            const auto scissor_rect_height = static_cast<uint32_t>(
                rx::algorithm::max(0.0F, round(cmd->clip_rect.h * framebuffer_size_ratio.y)));

            cmds.set_scissor_rect(scissor_rect_x, scissor_rect_y, scissor_rect_width, scissor_rect_height);

            cmds.draw_indexed_mesh(cmd->elem_count, offset);

            // Copy data into the vector of real vertices, adding in the texture ID
            for(uint32_t i = offset; i < offset + cmd->elem_count; i++) {
                const auto raw_vertex = raw_vertices[i];
                const auto tex_idx = 0; // *nk_tex_id_to_descriptor_idx.find(cmd->texture.id);

                vertices[i] = NuklearVertex{raw_vertex.position, raw_vertex.uv, raw_vertex.color, tex_idx};
            }

            offset += cmd->elem_count;
        }

        mesh->set_vertex_data(vertices.data(), vertices.size() * sizeof(NuklearVertex));
        mesh->set_index_data(indices.data(), indices.size() * sizeof(uint16_t));

        if(num_sets_used > 1) {
            logger->info("Used %u descriptor sets. Maybe you should only use one", num_sets_used);
        }

        materials += used_materials;
        used_materials.clear();

        clear_context();

        consume_input();
    }

    rx::pair<uint32_t, NuklearUiMaterial*> NuklearUiPass::get_next_material() {
        if(materials.is_empty()) {
            return renderer.create_material<NuklearUiMaterial>();

        } else {
            const auto ret_val = materials.last();
            materials.pop_back();

            return ret_val;
        }
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
