#pragma once

#include <cstdint>

namespace nova::bf {
    constexpr uint64_t MAX_VERTEX_BUFFER_SIZE = 512 * 1024;
    constexpr uint64_t MAX_INDEX_BUFFER_SIZE = 128 * 1024;

    constexpr const char* UI_PIPELINE_NAME = "BestFriendUI";

    constexpr const char* DEFAULT_TEXTURE_NAME = "BestFriendDefaultTexture";

    constexpr const char* SHADERS_PATH = "renderpacks/Simple/shaders";

    constexpr const char* BEST_FRIEND_GLOBALS_GROUP = "BestFriend";
} // namespace nova::bf
