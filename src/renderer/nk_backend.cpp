#include "nk_backend.hpp"

#include <nova_renderer/nova_renderer.hpp>

#define NK_IMPLEMENTATION
#include <nuklear.h>

#include "../../external/nova-renderer/external/glfw/include/GLFW/glfw3.h"
#include "../util/constants.hpp"

using namespace nova::renderer;
using namespace shaderpack;
using namespace rhi;

struct UiDrawParams {
    glm::mat4 projection;
};

struct NkVertex {
    glm::vec2 position;
    glm::vec2 uv;
    glm::u8vec4 color;
};

nk_buttons to_nk_mouse_button(uint32_t button);

NuklearDevice::NuklearDevice(NovaRenderer& renderer)
    : renderer(renderer), mesh(renderer.create_procedural_mesh(MAX_VERTEX_BUFFER_SIZE, MAX_INDEX_BUFFER_SIZE)) {

    // TODO: Some way to validate that this pass exists in the loaded shaderpack
    const FullMaterialPassName& ui_full_material_pass_name = {"UI", "Color"};
    StaticMeshRenderableData ui_renderable_data = {};
    ui_renderable_data.mesh = mesh.get_key();
    ui_renderable_id = renderer.add_renderable_for_material(ui_full_material_pass_name, ui_renderable_data);

    const auto window = renderer.get_window();
    window->register_key_callback([&](const auto& key, const bool is_press, const bool is_control_down, const bool /* is_shift_down */) {
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

std::shared_ptr<nk_context> NuklearDevice::get_context() const { return ctx; }

void NuklearDevice::begin_frame() {
    nk_input_begin(ctx.get());

    for(const auto& [key, is_pressed] : keys) {
        nk_input_key(ctx.get(), key, is_pressed);
    }

    keys.clear();

    nk_input_motion(ctx.get(), most_recent_mouse_position.x, most_recent_mouse_position.y);

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

void NuklearDevice::render() {}

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
