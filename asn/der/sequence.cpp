#include <Windows.h>

#include "asn/der/sequence.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

/*************************************************************************************************/
IASNSequence::IASNSequence(size_t count) : count(count) {
}

size_t IASNSequence::span() {
    size_t s = 1;
}
