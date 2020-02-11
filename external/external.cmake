# General
set(BUILD_STATIC_LIBS ON CACHE BOOL "Compile everything as a static lib" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Don't compile anything as a shared lib" FORCE)

# Nova
set(NOVA_TEST OFF CACHE BOOL "" FORCE)
set(NOVA_ENABLE_EXPERIMENTAL OFF CACHE BOOL "" FORCE)
set(NOVA_TREAT_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(NOVA_PACKAGE ON CACHE BOOL "" FORCE)
set(NOVA_ENABLE_OPENGL_RHI ON CACHE BOOL "" FORCE)
set(NOVA_ENABLE_VULKAN_RHI ON CACHE BOOL "" FORCE)
if(WIN32)
	set(NOVA_ENABLE_D3D12_RHI ON CACHE BOOL "" FORCE)
else()
	set(NOVA_ENABLE_D3D12_RHI OFF CACHE BOOL "" FORCE)
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/nova-renderer)


# TODO: Figure out how to invoke cargo and place things in the right place and everything
# 
# Commands I need to run:
# cargo run --bin bve-build

include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/bve-reborn/bve-native/include)

# N U K L E A R
add_compile_definitions(
	NK_INCLUDE_FIXED_TYPES 
	NK_INCLUDE_STANDARD_IO 
	NK_INCLUDE_STANDARD_VARARGS 
	NK_INCLUDE_DEFAULT_ALLOCATOR 
	NK_INCLUDE_VERTEX_BUFFER_OUTPUT 
	NK_INCLUDE_FONT_BAKING 
	NK_INCLUDE_DEFAULT_FONT 
	NK_KEYSTATE_BASED_INPUT)
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/Nuklear)

# EnTT
set(USE_LIBCPP OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/entt)
