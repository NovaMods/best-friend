#include "train_loading.hpp"

#include <bve.hpp>
#include <rx/core/log.h>
#include <rx/core/string.h>
#include <time.h>

#include "bve_wrapper.hpp"
#include "minitrace.h"
#include "rx/core/filesystem/file.h"

RX_LOG("TrainLoad", logger);

namespace bve {
    template <typename ValueType>
    ValueType* begin(const CVector<ValueType>& vec) {
        return vec.ptr;
    }

    template <typename ValueType>
    ValueType* end(const CVector<ValueType>& vec) {
        return vec.ptr + vec.count;
    }
} // namespace bve

namespace nova::bf {
    rx::string to_string(const bve::Mesh_Error& err) {
        switch(err.kind.tag) {
            case bve::Mesh_Error_Kind::Tag::UTF8:
                return "file encoding bad";

            case bve::Mesh_Error_Kind::Tag::OutOfBounds:
                return "out of bounds :(";

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

    rx::string to_string(const bve::Mesh_Warning& warning) {
        switch(warning.kind.tag) {
            case bve::Mesh_Warning_Kind::Tag::UselessInstruction:
                return rx::string::format("useless instruction %s", warning.kind.useless_instruction.name);

            default:
                return "unknown warning";
        }
    }

    template <typename MessageType>
    rx::string append_bve_messages(const bve::CVector<MessageType>& messages) {
        rx::string appended;

        // Should I SFINAE so that this loop doesn't give terrible error messages? probably. Am I going to? Absolutely not
        for(const auto& message : messages) {
            // to_string is on;ly defined for a few things. SFINAE could guard this but SFINAE can fuck off
            appended.append(rx::string::format("%s, ", to_string(message)));
        }

        // Strip off the last ", "
        return appended.substring(0, appended.size() - 2);
    }

    rx::optional<bve::Parsed_Static_Object> load_train_mesh(const rx::string& train_file_path) {
        MTR_SCOPE("load_train_mesh", "All");
        const auto train_file_string = [&] {
            MTR_SCOPE("load_train_mesh", "ReadFile");
            return g_bve->read_file_and_convert_to_utf8(train_file_path.data());
        }();

        if(train_file_string) {
            const auto train = [&] {
                MTR_SCOPE("load_train_mesh", "ParseFile");
                return g_bve->parse_mesh_from_string(train_file_string, bve::Mesh_File_Type::B3D);
            }();

            g_bve->delete_string(train_file_string);

            if(train.errors.count > 0) {
                const rx::string errors = append_bve_messages(train.errors);
                logger(rx::log::level::k_error, "Could not load train %s: %s", train_file_path, errors);

                return rx::nullopt;

            } else if(train.warnings.count > 0) {
                const auto& warnings = append_bve_messages(train.warnings);
                logger(rx::log::level::k_warning, "Encountered warnings while loading train %s: %s", train_file_path, warnings);

                logger(rx::log::level::k_info, "Loaded train %s", train_file_path);
                return train;

            } else {
                logger(rx::log::level::k_info, "Loaded train %s", train_file_path);
                return train;
            }
        } else {
            logger(rx::log::level::k_error, "Could not read train file %s", train_file_path);

            return rx::nullopt;
        }
    }
} // namespace nova::bf
