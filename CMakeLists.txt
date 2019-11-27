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

    src/ui/panel_manager.hpp
    src/ui/panel_manager.cpp

    src/ui/panel.hpp
    src/ui/train_selection_panel.hpp
    src/ui/train_selection_panel.cpp
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${BEST_FRIEND_SOURCES})

add_executable(best-friend ${BEST_FRIEND_SOURCES})
target_link_libraries(best-friend PRIVATE nova-renderer) # bve-native)  TODO: Figure out calling cargo and whatnot
target_include_directories(best-friend PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external/nova_renderer/include)
