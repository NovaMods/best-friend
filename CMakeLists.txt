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
if(NOT NOVA_PACKAGE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

#################################
# Setup 3rd party dependencies. #
#################################
include(external)

##########################
# Add viewer source code #
##########################
set(
    BEST_FRIEND_SOURCES
    src/main.cpp
)

add_executable(best-friend ${BEST_FRIEND_SOURCES})
target_link_libraries(best-friend PRIVATE nova-renderer)
target_include_directories(best-friend PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external/nova_renderer/include)

if(WIN32)
    # Visual Studio is stupid. Is it really that hard to use filesystem directories to organize code like literally
    # everyone else?
    foreach(source IN LISTS BEST_FRIEND_SOURCES)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" source_path_msvc "${source_path}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()
endif()