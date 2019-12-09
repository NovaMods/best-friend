#include "nk_backend.hpp"

#include <nova_renderer/frontend/procedural_mesh.hpp>
#include <nova_renderer/nova_renderer.hpp>

#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#include <nuklear.h>

#include "../../external/nova-renderer/external/glfw/include/GLFW/glfw3.h"
#include "../../external/nova-renderer/src/util/logger.hpp"
#include "../util/constants.hpp"
using namespace nova::renderer;
using namespace shaderpack;
using namespace rhi;

namespace nova::bf {
    constexpr PixelFormatEnum UI_IMAGE_FORMAT = PixelFormatEnum::RGBA8;

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
        : renderer(renderer), mesh(renderer.create_procedural_mesh(MAX_VERTEX_BUFFER_SIZE, MAX_INDEX_BUFFER_SIZE)) {

        // TODO: Some way to validate that this pass exists in the loaded shaderpack
        const FullMaterialPassName& ui_full_material_pass_name = {"UI", "Color"};
        StaticMeshRenderableData ui_renderable_data = {};
        ui_renderable_data.mesh = mesh.get_key();
        ui_renderable_id = renderer.add_renderable_for_material(ui_full_material_pass_name, ui_renderable_data);

        vertices.reserve(MAX_VERTEX_BUFFER_SIZE / sizeof(NuklearVertex));
        indices.reserve(MAX_INDEX_BUFFER_SIZE / sizeof(uint32_t));

        init_nuklear();

        register_input_callbacks();

        create_textures();

        renderer.register_ui_draw_function([&](CommandList* cmds) { render(cmds); });
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
        nk_input_motion(ctx.get(), most_recent_mouse_position.x, most_recent_mouse_position.y);

        // Consume the most recent mouse button
        if(most_recent_mouse_button) {
            nk_input_button(ctx.get(),
                            most_recent_mouse_button->first,
                            most_recent_mouse_position.x,
                            most_recent_mouse_position.y,
                            most_recent_mouse_button->second);
            most_recent_mouse_button = {};
        }

        nk_input_end(ctx.get());
    }

    void NuklearDevice::render(CommandList* cmds) {
        static const nk_draw_vertex_layout_element vertex_layout[] =
            {{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct NuklearVertex, position)},
             {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct NuklearVertex, uv)},
             {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct NuklearVertex, color)},
             {NK_VERTEX_LAYOUT_END}};

        nk_convert_config config = {};
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(NuklearVertex);
        config.vertex_alignment = NK_ALIGNOF(NuklearVertex);
        config.null = null;
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

            const int tex_index = cmd->texture.id;
            const auto tex_itr = textures.find(tex_index);
            if(current_descriptor_textures.size() < MAX_NUM_TEXTURES) {
                current_descriptor_textures.emplace_back(*tex_itr);

            } else {
                std::vector<DescriptorSetWrite> writes(1);
                DescriptorSetWrite& write = writes[0];
                write.set = *descriptor_set_itr;
                write.first_binding = 0;
                write.type = DescriptorType::CombinedImageSampler;
                write.bindings.reserve(current_descriptor_textures.size());

                std::transform(current_descriptor_textures.begin(),
                               current_descriptor_textures.end(),
                               std::back_insert_iterator<std::vector<DescriptorResourceInfo>>(write.bindings),
                               [&](Image* image) {
                                   DescriptorResourceInfo info = {};
                                   info.image_info.image = image;
                                   info.image_info.format.pixel_format = UI_IMAGE_FORMAT;
                                   info.image_info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
                                   info.image_info.format.width = FONT_ATLAS_TEXTURE_WIDTH;
                                   info.image_info.format.height = FONT_ATLAS_TEXTURE_HEIGHT;
                                   return info;
                               });

                renderer.get_engine()->update_descriptor_sets(writes);

                num_sets_used++;
                ++descriptor_set_itr;
            }
        }

        NOVA_LOG(INFO) << "Used " << num_sets_used << " descriptor sets. Maybe you should only use one";

        const auto [verts, indices] = mesh->get_buffers_for_frame(renderer.get_current_frame_index());

        cmds->bind_descriptor_sets(sets, nullptr); // TODO: Command list should store the pipeline interface internally 
        cmds->bind_vertex_buffers({verts, verts, verts});
        cmds->bind_index_buffer(indices);
        cmds->draw_indexed_mesh(35, 1); // TODO: Figure out how many indices
    }

    void NuklearDevice::init_nuklear() {
        nk_buffer_init_default(&nk_cmds);

        nk_buffer_init_fixed(&vertex_buffer, vertices.data(), MAX_VERTEX_BUFFER_SIZE);
        nk_buffer_init_fixed(&index_buffer, indices.data(), MAX_INDEX_BUFFER_SIZE);
    }

    void NuklearDevice::create_texture() { shaderpack::TextureCreateInfo; }

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
        }
    }

} // namespace nova::bf
