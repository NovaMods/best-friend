#pragma once
#include <mutex>
#include <optional>

#include <nova_renderer/renderables.hpp>
#include <nova_renderer/util/container_accessor.hpp>
#include <nuklear.h>

//! \brief Nuklear backend that renders Nuklear geometry with the Nova renderer
//!
//! Based on the Nuklear GL4 examples

namespace nova {
    namespace renderer {
        class NovaRenderer;

        class ProceduralMesh;

        namespace rhi {
            struct Buffer;
            struct Pipeline;
        } // namespace rhi
    }     // namespace renderer

    namespace bf {
        struct NuklearVertex {
            glm::vec2 position;
            glm::vec2 uv;
            glm::u8vec4 color;
        };

        /*!
         * \brief Renders the Nuklear UI
         */
        class NuklearDevice final {
        public:
            void init_nuklear();
            explicit NuklearDevice(renderer::NovaRenderer& renderer);

            ~NuklearDevice();

            std::shared_ptr<nk_context> get_context() const;

            /*!
             * \brief Begins a frame by doing things like input handling
             */
            void consume_input();

            /*!
             * \brief Renders all the UI elements that were drawn to the context
             */
            void render();

        private:
            std::shared_ptr<nk_context> ctx;

            renderer::NovaRenderer& renderer;

            renderer::rhi::Buffer* ui_draw_params;

            renderer::RenderableId ui_renderable_id;

			nk_buffer cmds;

            renderer::MapAccessor<renderer::MeshId, renderer::ProceduralMesh> mesh;
            std::vector<NuklearVertex> vertices;
			nk_buffer vertex_buffer;
            std::vector<uint32_t> indices;
			nk_buffer index_buffer;

			nk_draw_null_texture null;

            std::mutex key_buffer_mutex;

            /*!
             * \brief Vector of all the keys we received this frame and if they were pressed down or not
             */
            std::vector<std::pair<nk_keys, bool>> keys;

            glm::dvec2 most_recent_mouse_position;

            std::optional<std::pair<nk_buttons, bool>> most_recent_mouse_button;
        };
    } // namespace bf
} // namespace nova