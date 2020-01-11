#pragma once

#include <string>

#include "asn/identifier.hpp"

namespace WarGrey::DTPM {
    typedef std::basic_string<unsigned char> octets;

    WarGrey::DTPM::octets asn_length_to_octets(size_t length);
    size_t asn_octets_to_length(WarGrey::DTPM::octets& length, size_t* offset = nullptr);

    bool asn_primitive_predicate(WarGrey::DTPM::ASNPrimitive type, WarGrey::DTPM::octets& content, size_t offset = 0);
    WarGrey::DTPM::octets asn_octets_box(uint8 tag, WarGrey::DTPM::octets& content);
    size_t asn_octets_unbox(WarGrey::DTPM::octets& basn, size_t* offset);
}
