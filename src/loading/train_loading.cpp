#include "train_loading.hpp"

#include <bve.hpp>
#include <rx/core/log.h>
#include <rx/core/string.h>
#include <time.h>

#include "rx/core/filesystem/file.h"
#include "rx/core/profiler.h"
#include "minitrace.h"
#include "bve_wrapper.hpp"
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

    bool has_real_errors(const bve::CVector<bve::Mesh_Error>& potential_errors) {
        for(uint32_t i = 0; i < potential_errors.count; i++) {
            const auto& potential_error = potential_errors.ptr[i];
            if(potential_error.kind.tag == bve::Mesh_Error_Kind::Tag::UTF8 ||
               potential_error.kind.tag == bve::Mesh_Error_Kind::Tag::OutOfBounds ||
               potential_error.kind.tag == bve::Mesh_Error_Kind::Tag::GenericCSV ||
               potential_error.kind.tag == bve::Mesh_Error_Kind::Tag::UnknownCSV) {
                return true;
            }
        }
        return false;
    }

    rx::optional<bve::Parsed_Static_Object> load_train_mesh(const rx::string& train_file_path) {
        MTR_SCOPE("load_train_mesh", "All");
        const auto data = [&] {
            MTR_SCOPE("load_train_mesh", "ReadFile");
            return rx::filesystem::read_text_file(train_file_path);
        }();
        
        if(data) {
            rx::string file_contents = [&] {
                MTR_SCOPE("load_train_mesh", "VectorToString");
                return rx::string{reinterpret_cast<const char*>(data->data())};
            }();

            const auto train = [&] {
                MTR_SCOPE("load_train_mesh", "BVE");
                return g_bve->parse_mesh_from_string(file_contents.data(), bve::Mesh_File_Type::B3D);
            }();

            if(has_real_errors(train.errors)) {
                rx::string errors;
                for(uint32_t i = 0; i < train.errors.count; i++) {
                    errors.append(to_string(train.errors.ptr[i]));
                    if(i < train.errors.count - 1) {
                        errors.append(", ");
                    }
                }

                logger(rx::log::level::k_error, "Could not load train %s: %s", train_file_path, errors);

                return rx::nullopt;

            } else {
                logger(rx::log::level::k_info, "Successfully loaded train %s", train_file_path);

                return train;
            }
        } else {
            logger(rx::log::level::k_error, "Could not read train file %s", train_file_path);

            return rx::nullopt;
        }
    }
} // namespace nova::bf
