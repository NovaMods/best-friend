#include <iostream>

#include <nova_renderer/nova_renderer.hpp>

int main(int argc, const char** argv) {
	std::cout << "Hello, world!";

	nova::renderer::NovaSettings settings;
#ifndef NDEBUG
	settings.debug.enabled = true;
#endif

	settings.window.title = "Best Friend Train Viewer";
	settings.window.width = 640;
	settings.window.height = 480;

#if WIN32
	settings.api = nova::renderer::GraphicsApi::D3D12;
#else
	settings.api = nova::renderer::GraphicsApi::Vulkan;
#endif
	
	nova::renderer::NovaRenderer renderer(settings);
	
	
	return 0;
}