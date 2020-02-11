#include "train_loading.hpp"

#include <bve.hpp>
#include <rx/core/log.h>
#include <rx/core/string.h>

RX_LOG("TrainLoad", logger);

namespace nova::bf {
    rx::string to_string(const bve::Mesh_Error err) {
        switch(err.kind.tag) {
            case bve::Mesh_Error_Kind::Tag::UTF8:
                return "file encoding bad";

            case bve::Mesh_Error_Kind::Tag::OutOfBounds:
                return "out of bounds :(";

            case bve::Mesh_Error_Kind::Tag::UselessInstruction:
                return rx::string::format("useless instruction %s", err.kind.useless_instruction.name);

            case bve::Mesh_Error_Kind::Tag::UnknownInstruction:
                return rx::string::format("unknown instruction %s", err.kind.unknown_instruction.name);

            case bve::Mesh_Error_Kind::Tag::GenericCSV:
                return rx::string::format("CSV error %s", err.kind.generic_csv.msg);

            case bve::Mesh_Error_Kind::Tag::UnknownCSV:
                return "unknown CSV %s";

            default:
                return "unknown error";
        }
    }

    void load_train_mesh(const rx::string& train_file_path) {
        const auto train = bve_parse_mesh_from_string(train_file_path.data(), bve::Mesh_File_Type::B3D);
        if(train.errors.count > 0) {
            rx::string errors;
            for(uint32_t i = 0; i < train.errors.count; i++) {
                errors.append(to_string(train.errors.ptr[i]));
                if(i < train.errors.count - 1) {
                    errors.append(", ");
                }
            }

            logger(rx::log::level::k_error, "Could not load train %s: %s", train_file_path, errors);

        } else {
            logger(rx::log::level::k_info, "Successfully loaded train %s", train_file_path);
        }

        bve_delete_parsed_static_object(train);
    }
} // namespace nova::bf
