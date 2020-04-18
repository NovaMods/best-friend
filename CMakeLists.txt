cmake_minimum_required(VERSION 3.14)
project(best-friend)

############################
# Initialize cmake options #
############################
set(3RD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/external)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tools/cmake" "${3RD_PARTY_DIR}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#################################
# Setup 3rd party dependencies. #
#################################
include(external)

##########################
# Add viewer source code #
##########################
set(
    BEST_FRIEND_SOURCES

	external/Nuklear/nuklear.h

    src/main.cpp

    src/util/constants.hpp
	
    src/ec/component.hpp
    src/ec/transform.hpp
    src/ec/system.hpp
		
    src/ui/panel_manager.hpp
    src/ui/panel_manager.cpp
    src/ui/panel.hpp
    src/ui/panel.cpp
    src/ui/train_selection_panel.hpp
    src/ui/train_selection_panel.cpp
    src/ui/ui_events.hpp
    src/ui/ui_events.cpp

	src/loading/train_loading.hpp
	src/loading/train_loading.cpp
	src/loading/data_loader.hpp
	src/loading/data_loader.cpp
	src/loading/bve_wrapper.hpp
	src/loading/bve_wrapper.cpp
	
    src/renderer/nk_backend.hpp
    src/renderer/nk_backend.cpp
    src/renderer/camera_component.hpp
    src/renderer/camera_component.cpp
    src/renderer/train_rendering.hpp
    src/renderer/train_rendering.cpp

    src/input/input.hpp
    src/input/input.cpp
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${BEST_FRIEND_SOURCES})

add_executable(best-friend ${BEST_FRIEND_SOURCES})
target_link_directories(best-friend PRIVATE "${CMAKE_CURRENT_LIST_DIR}/external/bve-reborn/target/release")
target_link_libraries(best-friend PRIVATE nova-renderer bve_native.dll.lib EnTT::EnTT) 
target_include_directories(best-friend PRIVATE "${CMAKE_CURRENT_LIST_DIR}/external/nova_renderer/include")
target_compile_definitions(best-friend PRIVATE _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(best-friend PRIVATE BEST_FRIEND_DATA_DIR="${CMAKE_CURRENT_LIST_DIR}/data")
target_compile_definitions(best-friend INTERFACE NOVA_DEBUG)

###########################
# Compile builtin shaders #
###########################
add_dependencies(best-friend shader-compiler)

find_package(Python3)
if(NOT Python3_Interpreter_FOUND)
    message(ERROR "Could not find Python3 interpreter")
endif()

add_custom_command(TARGET best-friend
    PRE_BUILD
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/external/nova-renderer/tools/compile_shaders.py" "${CMAKE_BINARY_DIR}/shader-compiler.exe" "${CMAKE_CURRENT_LIST_DIR}/data/renderpacks/Simple/shaders" "${CMAKE_CURRENT_LIST_DIR}/data/renderpacks/Simple/shaders"
)
