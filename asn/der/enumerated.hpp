#pragma once

#include "asn/der/base.hpp"
#include "asn/der/identifier.hpp"

namespace WarGrey::DTPM {
#define define_asn_enum(E, e, ...) \
private enum class E { __VA_ARGS__ }; \
inline size_t asn_##e##_span(E e) { return asn_enum_span(e); } \
inline WarGrey::DTPM::octets asn_##e##_to_octets(E e) { return asn_enum_to_octets(e); } \
inline size_t asn_##e##_into_octets(E e, uint8* octets, size_t offset = 0) { return asn_enum_into_octets(e, octets, offset); } \
inline E asn_octets_to_##e(WarGrey::DTPM::octets& benum, size_t* offset = nullptr) { return asn_octets_to_enum<E>(benum, offset); }

    template<typename E>
    inline size_t asn_enum_span(E enumerated) {
        return asn_fixnum_span(static_cast<long long>(enumerated));
    }

    template<typename E>
    inline WarGrey::DTPM::octets asn_enum_to_octets(E enumerated) {
        return asn_int64_to_octets(static_cast<long long>(enumerated), ASNPrimitive::Enumerated);
    }

    template<typename E>
    inline size_t asn_enum_into_octets(E enumerated, uint8* octets, size_t offset = 0) {
        return asn_int64_into_octets(static_cast<long long>(enumerated), octets, offset, ASNPrimitive::Enumerated);
    }

    template<typename E>
    inline E asn_octets_to_enum(WarGrey::DTPM::octets& benum, size_t* offset = nullptr) {
        return static_cast<E>(asn_octets_to_fixnum(benum, offset));
    }
}
