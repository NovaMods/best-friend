#pragma once

#include <bve.hpp>
#include <rx/core/global.h>

namespace nova::bf {
    class BveWrapper {
    public:
        BveWrapper();

        void set_panic_data(void* data);

        void set_panic_handler(const bve::PanicHandler& handler);

        [[nodiscard]] char* read_file_and_convert_to_utf8(const char* filename);

        [[nodiscard]] bve::Parsed_Static_Object parse_mesh_from_string(const char* string, bve::Mesh_File_Type file_type);

        void delete_parsed_static_object(bve::Parsed_Static_Object&& object);

        void delete_string(char* ptr);

        [[nodiscard]] void* get_panic_data();

        [[nodiscard]] bve::PanicHandler get_panic_handler();
    };

    class TextureSet {
    public:
        explicit TextureSet(bve::Texture_Set* set);

        [[nodiscard]] size_t add(const char* value) const;

        [[nodiscard]] size_t len() const;

        [[nodiscard]] const char* operator[](size_t idx) const;

    private:
        bve::Texture_Set* set;
    };

    extern RX_GLOBAL<BveWrapper> g_bve;
} // namespace nova::bf
