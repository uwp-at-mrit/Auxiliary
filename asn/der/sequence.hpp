#pragma once

#include "asn/der/base.hpp"

namespace WarGrey::DTPM {
    private class IASNSequence {
    public:
        WarGrey::DTPM::octets to_octets();
        void from_octets(WarGrey::DTPM::octets& basn, size_t* offset = nullptr);
    };
}
