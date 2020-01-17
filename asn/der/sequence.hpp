#pragma once

#include "asn/der/base.hpp"

namespace WarGrey::DTPM {
    private class IASNSequence {
    public:
        static bool predicate(WarGrey::DTPM::octets& content, size_t offset = 0);

    public:
        virtual ~IASNSequence() noexcept {}
        IASNSequence(size_t count);

    public:
        size_t span();
        WarGrey::DTPM::octets to_octets();
        void from_octets(WarGrey::DTPM::octets& octets, size_t* offset = nullptr);

    protected:
        size_t span_ref(size_t idx);

    private:
        size_t count;
    };
}
