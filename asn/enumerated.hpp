#pragma once

#include "asn/base.hpp"
#include "asn/identifier.hpp"

namespace WarGrey::DTPM {
#define define_asn_enum(E, e, ...) \
private enum class E { __VA_ARGS__ }; \
inline WarGrey::DTPM::octets asn_##e##_to_octets(E e) { return asn_enum_to_octets(e); } \
inline E asn_octets_to_##e(WarGrey::DTPM::octets& benum, size_t* offset = nullptr) { return asn_octets_to_enum<E>(benum, offset); }

    template<typename E>
    inline WarGrey::DTPM::octets asn_enum_to_octets(E enumerated) {
        return asn_int64_to_octets(static_cast<long long>(enumerated), ASNPrimitive::Enumerated);
    }

    template<typename E>
    inline E asn_octets_to_enum(WarGrey::DTPM::octets& benum, size_t* offset = nullptr) {
        return static_cast<E>(asn_octets_to_fixnum(benum, offset));
    }
}
