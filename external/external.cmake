# General
set(BUILD_STATIC_LIBS ON CACHE BOOL "Compile everything as a static lib" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Don't compile anything as a shared lib" FORCE)

# Nova
set(NOVA_TEST OFF CACHE BOOL "" FORCE)
set(NOVA_ENABLE_EXPERIMENTAL OFF CACHE BOOL "" FORCE)
set(NOVA_TREAT_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(NOVA_PACKAGE ON CACHE BOOL "" FORCE)
if(WIN32)
	set(NOVA_ENABLE_D3D12_RHI ON CACHE BOOL "" FORCE)
	set(NOVA_ENABLE_VULKAN_RHI OFF CACHE BOOL "" FORCE)
else()
	set(NOVA_ENABLE_D3D12_RHI OFF CACHE BOOL "" FORCE)
	set(NOVA_ENABLE_VULKAN_RHI ON CACHE BOOL "" FORCE)
endif()

set(NOVA_ENABLE_OPENGL_RHI OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/nova-renderer)


# TODO: Figure out how to invoke cargo and place things in the right place and everything
# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake-cargo/cmake)
# include(Cargo)

# add_crate(${CMAKE_CURRENT_LIST_DIR}/bve-reborn/bve-native/Cargo.toml)

# N U K L E A R
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/Nuklear)
