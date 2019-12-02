#pragma once

//! \brief Nuklear backend that renders Nuklear geometry with the Nova renderer
//!
//! Based on the Nuklear GL4 examples

struct nk_context;

namespace nova::renderer {
    class NovaRenderer;

    namespace rhi {
        class Buffer;
        class Pipeline;
    }
} // namespace nova::renderer

/*!
 * \brief Renders the Nuklear UI
 */
class NuklearDevice final {
public:
    explicit NuklearDevice(nova::renderer::NovaRenderer& renderer);

    nk_context* make_context();

    /*!
     * \brief Renders all the UI elements that were drawn to the context
     * 
     * \param ctx The Nuklear context to render
     */
    void render(nk_context* ctx);

private:
    nova::renderer::NovaRenderer& renderer;

    nova::renderer::rhi::Buffer* vertex_buffer;
    nova::renderer::rhi::Buffer* index_buffer;
    nova::renderer::rhi::Buffer* ui_draw_params;

    nova::renderer::rhi::Pipeline* pipeline;

    nova::renderer::rhi::Pipeline* make_pipeline();
};
