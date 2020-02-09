#pragma once

#include <nova_renderer/renderables.hpp>
#include <nova_renderer/rhi/forward_decls.hpp>
#include <nova_renderer/ui_renderer.hpp>
#include <nova_renderer/util/container_accessor.hpp>
#include <nuklear.h>

#include "nova_renderer/resource_loader.hpp"

//! \brief Nuklear backend that renders Nuklear geometry with the Nova renderer
//!
//! Based on the Nuklear GL4 examples

namespace nova {
    namespace renderer {
        class NovaRenderer;

        class ProceduralMesh;

    } // namespace renderer

    namespace bf {
        struct NuklearImage {
            renderer::TextureResourceAccessor image;

            struct nk_image nk_image;

            NuklearImage(renderer::TextureResourceAccessor image, struct nk_image nk_image = {});
        };

        struct NullNuklearImage : NuklearImage {
            nk_draw_null_texture nk_null_tex = {};

            NullNuklearImage(const renderer::TextureResourceAccessor& image,
                             struct nk_image nk_image = {},
                             nk_draw_null_texture null_tex = {});
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

            [[nodiscard]] rx::optional<NuklearImage> create_image(const rx::string& name,
                                                                  rx_size width,
                                                                  rx_size height,
                                                                  const void* image_data);

            void clear_context() const;

            static renderer::shaderpack::RenderPassCreateInfo get_create_info();

            void write_textures_to_descriptor(renderer::FrameContext& frame_ctx,
                                              const rx::vector<renderer::rhi::Image*>& current_descriptor_textures);

        private:
            std::shared_ptr<nk_context> ctx;

            renderer::NovaRenderer& renderer;

            glm::vec2 framebuffer_size_ratio{};

            renderer::rhi::Buffer* ui_draw_params = nullptr;

            nk_buffer nk_cmds{};

            renderer::MapAccessor<renderer::MeshId, renderer::ProceduralMesh> mesh;
            rx::vector<NuklearVertex> vertices;
            nk_buffer nk_vertex_buffer{};
            rx::vector<uint16_t> indices;
            nk_buffer nk_index_buffer{};

            std::mutex key_buffer_mutex;

            /*!
             * \brief Vector of all the keys we received this frame and if they were pressed down or not
             */
            rx::vector<std::pair<nk_keys, bool>> keys;

            glm::dvec2 most_recent_mouse_position{};

            rx::optional<std::pair<nk_buttons, bool>> most_recent_mouse_button;

            renderer::rhi::DescriptorPool* pool = nullptr;

            /*!
             * \brief Descriptor sets for the UI material
             *
             * The UI material doesn't _actually_ exist, because rendering UI is hard, but if there was a real UI material, these
             * descriptors would be for that
             */
            rx::array<rx::vector<renderer::rhi::DescriptorSet*>[renderer::NUM_IN_FLIGHT_FRAMES]> material_descriptors;

            rx::map<int, renderer::TextureResourceAccessor> textures;
            uint32_t next_image_idx = 0;

            nk_font_atlas* nk_atlas;
            std::unique_ptr<NullNuklearImage> null_texture;
            std::unique_ptr<NuklearImage> font_image;
            nk_font* font{};

            rx::memory::allocator* allocator;

            void init_nuklear();

            void create_resources();

            void create_null_texture();

            void create_ui_ubo();

            void load_font();

            void retrieve_font_atlas();

            void register_input_callbacks();

            void save_framebuffer_size_ratio();

            void create_descriptor_sets(const renderer::Pipeline& pipeline, uint32_t frame_idx);

        protected:
            /*!
             * \brief Uploads vertex data for this frame's UI
             */
            void setup_renderpass(renderer::rhi::CommandList& cmds, renderer::FrameContext& frame_ctx) override;

            /*!
             * \brief Renders all the UI elements that were drawn to the context
             */
            void render_ui(renderer::rhi::CommandList& cmds, renderer::FrameContext& frame_ctx) override;
        };
    } // namespace bf
} // namespace nova