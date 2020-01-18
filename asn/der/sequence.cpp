#include <Windows.h>

#include "asn/der/sequence.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

/*************************************************************************************************/
IASNSequence::IASNSequence(size_t count) : count(count) {}

size_t IASNSequence::span() {
    size_t s = 0;

    for (size_t idx = 0; idx < this->count; idx++) {
        s += this->span_ref(idx);
    }

    return 1 + asn_length_span(s) + s;
}

octets IASNSequence::to_octets() {
    octets o;

    return o;
}
