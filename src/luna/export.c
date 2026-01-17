#include "parse.h"

Vec(u8) export_flat_binary(Section** sections, usize len) {
    usize binary_length = 0;
    // for_n(i, 0, len) {
    //     Section* section = sections[i];
        
    //     binary_length = max(binary_length, section->map_address + vec_len(section->bytes));
    // }

    Vec(u8) output = vec_new(u8, binary_length);
    // vec_len(output) = binary_length;

    // for_n(i, 0, len) {
    //     Section* section = sections[i];
    //     memcpy(&output[section->map_address], section->bytes, vec_len(section->bytes));
    // }

    return output;
}
