#pragma once
#include "nova_renderer/util/container_accessor.hpp"
#include "nova_renderer/renderables.hpp"
#include <mutex>
// #include "nova_renderer/frontend/procedural_mesh.hpp"

//! \brief Nuklear backend that renders Nuklear geometry with the Nova renderer
//!
//! Based on the Nuklear GL4 examples

struct nk_context;

namespace nova::renderer {
    class NovaRenderer;

    class ProceduralMesh;

    namespace rhi {
        struct Buffer;
        struct Pipeline;
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
     * \brief Begins a frame by doing things like input handling
     */
    void begin_frame();

    /*!
     * \brief Renders all the UI elements that were drawn to the context
     */
    void render();

private:
    nk_context* ctx;

    nova::renderer::NovaRenderer& renderer;

    nova::renderer::rhi::Buffer* ui_draw_params;

    nova::renderer::RenderableId ui_renderable_id;

    nova::renderer::MapAccessor<nova::renderer::MeshId, nova::renderer::ProceduralMesh> mesh;

    std::mutex key_buffer_mutex;

    /*!
     * \brief Vector of all the keys we received this frame and if they were pressed down or not
     */
    std::vector<std::pair<uint32_t, bool>> keys;
};
