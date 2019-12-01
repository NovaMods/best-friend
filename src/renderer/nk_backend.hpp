#pragma once

//! \brief Nuklear backend that renders Nuklear geometry with the Nova renderer
//!
//! Based on the Nuklear GL4 examples

struct nk_context;

namespace nova::renderer {
    class NovaRenderer;
}

nk_context nk_nova_init(nova::renderer::NovaRenderer& renderer);
void nk_nova_shutdown();

void nk_nova_new_frame();
void nk_nova_render();
