#pragma once
#include <bve.hpp>
#include <rx/core/optional.h>

namespace rx {
    struct string;
}

namespace nova::bf {
    rx::optional<bve::Parsed_Static_Object> load_train_mesh(const rx::string& train_file_path);
}
