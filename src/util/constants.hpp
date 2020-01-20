#pragma once

#include <cstdint>

namespace nova::bf {
    constexpr uint64_t MAX_VERTEX_BUFFER_SIZE = 512 * 1024;
    constexpr uint64_t MAX_INDEX_BUFFER_SIZE = 128 * 1024;
    constexpr uint64_t MAX_NUM_TEXTURES = 256;

    const std::string UI_PIPELINE_NAME = "BestFriendUI";

    const std::string NULL_TEXTURE_NAME = "BestFriendNull";

    const std::string SHADERS_PATH = "shaderpacks/Simple/shaders";
} // namespace nova::bf
