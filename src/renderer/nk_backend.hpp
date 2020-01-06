#pragma once
#include <mutex>
#include <optional>
#include <unordered_map>

#include <nova_renderer/frontend/ui_renderer.hpp>
#include <nova_renderer/memory/allocators.hpp>
#include <nova_renderer/renderables.hpp>
#include <nova_renderer/util/container_accessor.hpp>
#include <nuklear.h>

#include "nova_renderer/frontend/resource_loader.hpp"

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
        struct NuklearImage {
            renderer::TextureResourceAccessor image;

            struct nk_image nk_image;

            NuklearImage(renderer::TextureResourceAccessor image, struct nk_image nk_image = {});
        };

        struct NullNuklearImage : NuklearImage {
            nk_draw_null_texture nk_null_tex = {};

            NullNuklearImage(const renderer::TextureResourceAccessor& image, struct nk_image nk_image = {}, nk_draw_null_texture null_tex = {});
        };

        struct NuklearVertex {
            glm::vec2 position;
            glm::vec2 uv;
            glm::u8vec4 color;
        };

        enum class ImageId {
            FontAtlas,

            Count, // Must always be last
        };

        /*!
         * \brief Renders the Nuklear UI
         */
        class NuklearDevice final : public renderer::UiRenderpass {
        public:
            explicit NuklearDevice(renderer::NovaRenderer& renderer);

            ~NuklearDevice();

            [[nodiscard]] std::shared_ptr<nk_context> get_context() const;

            /*!
             * \brief Begins a frame by doing things like input handling
             */
            void consume_input();

            [[nodiscard]] std::optional<NuklearImage> create_image(const std::string& name,
                                                                   std::size_t width,
                                                                   std::size_t height,
                                                                   const void* image_data);

        private:
            std::shared_ptr<nk_context> ctx;

            renderer::NovaRenderer& renderer;

            glm::vec2 framebuffer_size_ratio;

            renderer::rhi::Buffer* ui_draw_params;

            renderer::RenderableId ui_renderable_id;

            nk_buffer nk_cmds;

            renderer::MapAccessor<renderer::MeshId, renderer::ProceduralMesh> mesh;
            std::vector<NuklearVertex> vertices;
            nk_buffer nk_vertex_buffer;
            std::vector<uint32_t> indices;
            nk_buffer nk_index_buffer;

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
            std::unordered_map<int, renderer::TextureResourceAccessor> textures;
            uint32_t next_image_idx = static_cast<uint32_t>(ImageId::Count);

            std::unique_ptr<nk_font_atlas> nk_atlas;
            std::unique_ptr<NullNuklearImage> null_texture;
            std::unique_ptr<NuklearImage> font_image;
            nk_font* font;
            renderer::rhi::Pipeline* pipeline;
            renderer::rhi::PipelineInterface* pipeline_interface;

            std::unique_ptr<mem::AllocatorHandle<>> allocator;

            void init_nuklear();

            void create_textures();

            void create_pipeline();

            void load_font();

            void retrieve_font_atlas();

            void register_input_callbacks();

            void save_framebuffer_size_ratio();

        protected:
            /*!
             * \brief Renders all the UI elements that were drawn to the context
             */
            void render_ui(renderer::rhi::CommandList* cmds, renderer::FrameContext& frame_ctx) override;
        };
    } // namespace bf
} // namespace nova