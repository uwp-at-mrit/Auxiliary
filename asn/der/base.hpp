#pragma once

#include <string>

#include "datum/natural.hpp"

#include "asn/der/identifier.hpp"

namespace WarGrey::DTPM {
    typedef std::basic_string<unsigned char> octets;

    size_t asn_length_span(size_t length);
    WarGrey::DTPM::octets asn_length_to_octets(size_t length);
    size_t asn_octets_to_length(WarGrey::DTPM::octets& length, size_t* offset = nullptr);
    
    bool asn_primitive_predicate(WarGrey::DTPM::ASNPrimitive type, WarGrey::DTPM::octets& content, size_t offset = 0);
    WarGrey::DTPM::octets asn_octets_box(uint8 tag, WarGrey::DTPM::octets& content, size_t size);
    size_t asn_octets_unbox(WarGrey::DTPM::octets& basn, size_t* offset = nullptr);
    WarGrey::DTPM::octets asn_int64_to_octets(long long integer, WarGrey::DTPM::ASNPrimitive id = ASNPrimitive::Integer);

    // NOTE: `asn_octets_to_xxx` does not check the tag, please ensure that the octet is really what it should be.
    size_t asn_boolean_span(bool b);
    WarGrey::DTPM::octets asn_boolean_to_octets(bool b);
    bool asn_octets_to_boolean(WarGrey::DTPM::octets& bbool, size_t* offset = nullptr);

    size_t asn_fixnum_span(long long integer);
    inline WarGrey::DTPM::octets asn_fixnum_to_octets(long long integer) { return asn_int64_to_octets(integer, ASNPrimitive::Integer); }
    long long asn_octets_to_fixnum(WarGrey::DTPM::octets& bint, size_t* offset = nullptr);

    size_t asn_natural_span(WarGrey::DTPM::Natural& nat);
    WarGrey::DTPM::octets asn_natural_to_octets(WarGrey::DTPM::Natural& nat);
    WarGrey::DTPM::Natural asn_octets_to_natural(WarGrey::DTPM::octets& bnat, size_t* offset = nullptr);

    size_t asn_null_span(std::nullptr_t placeholder);
    WarGrey::DTPM::octets asn_null_to_octets(std::nullptr_t placeholder);
    std::nullptr_t asn_octets_to_null(WarGrey::DTPM::octets& bbool, size_t* offset = nullptr);

    size_t asn_real_span(double real);
    WarGrey::DTPM::octets asn_real_to_octets(double real);
    double asn_octets_to_real(WarGrey::DTPM::octets& breal, size_t* offset = nullptr);

    size_t asn_ia5_span(std::string& ia5_str);
    WarGrey::DTPM::octets asn_ia5_to_octets(std::string& ia5_str);
    std::string asn_octets_to_ia5(WarGrey::DTPM::octets& bia5, size_t* offset = nullptr);

    size_t asn_utf8_span(Platform::String^ wide_str);
    size_t asn_utf8_span(std::wstring& wide_str);
    WarGrey::DTPM::octets asn_utf8_to_octets(Platform::String^ wide_str);
    WarGrey::DTPM::octets asn_utf8_to_octets(std::wstring& wide_str);
    std::wstring asn_octets_to_utf8(WarGrey::DTPM::octets& butf8, size_t* offset = nullptr);

    template<typename Span, typename T>
    inline size_t asn_span(Span span, T v) { size_t size = span(v); return 1 + asn_length_span(size) + size; }
}
