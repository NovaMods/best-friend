#include "bve_wrapper.hpp"

#include <bve.hpp>

using namespace bve;

namespace nova::bf {
    RX_GLOBAL<BveWrapper> g_bve{"BestFriend", "BVE"};

    BveWrapper::BveWrapper() { bve_init(); }

    void BveWrapper::set_panic_data(void* data) { bve_set_panic_data(data); }

    void BveWrapper::set_panic_handler(const PanicHandler& handler) { bve_set_panic_handler(handler); }

    char* BveWrapper::read_file_and_convert_to_utf8(const char* filename) {
        return bve_filesystem_read_convert_utf8(filename);
    }

    Parsed_Static_Object BveWrapper::parse_mesh_from_string(const char* string, const Mesh_File_Type file_type) {
        return bve_parse_mesh_from_string(string, file_type);
    }

    void BveWrapper::delete_parsed_static_object(Parsed_Static_Object&& object) { bve_delete_parsed_static_object(object); }

    void BveWrapper::delete_string(char* ptr) { bve_delete_string(ptr); }

    void* BveWrapper::get_panic_data() { return bve_get_panic_data(); }

    PanicHandler BveWrapper::get_panic_handler() { return bve_get_panic_handler(); }

    TextureSet::TextureSet(Texture_Set* set) : set(set) {}

    size_t TextureSet::add(const char* value) const { return BVE_Texture_Set_add(set, value); }

    size_t TextureSet::len() const { return BVE_Texture_Set_len(set); }

    const char* TextureSet::operator[](const size_t idx) const { return BVE_Texture_Set_lookup(set, idx); }
} // namespace nova::bf
