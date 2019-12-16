#pragma once
#include <mutex>
#include <optional>
#include <unordered_map>

#include <nova_renderer/frontend/ui_renderer.hpp>
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
            class CommandList;

            struct Buffer;
            struct DescriptorSet;
            struct Image;
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
        class NuklearDevice final : public renderer::UiRenderpass {
        public:
            void register_input_callbacks();
            explicit NuklearDevice(renderer::NovaRenderer& renderer);

            ~NuklearDevice();

            std::shared_ptr<nk_context> get_context() const;

            /*!
             * \brief Begins a frame by doing things like input handling
             */
            void consume_input();

        private:
            std::shared_ptr<nk_context> ctx;

            renderer::NovaRenderer& renderer;

            renderer::rhi::Buffer* ui_draw_params;

            renderer::RenderableId ui_renderable_id;

            nk_buffer nk_cmds;

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

            /*!
             * \brief List of all the descriptor sets that can hold an array of textures
             */
            std::vector<renderer::rhi::DescriptorSet*> sets;
            std::unordered_map<int, renderer::rhi::Image*> textures;

            void init_nuklear();
            void create_textures();

            /*!
             * \brief Renders all the UI elements that were drawn to the context
             */
            void render(renderer::rhi::CommandList* cmds);

        protected:
            void render_ui(renderer::rhi::CommandList* cmds, renderer::FrameContext& frame_ctx) override;
        };
    } // namespace bf
} // namespace nova