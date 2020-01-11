#include "asn/base.hpp"

using namespace WarGrey::DTPM;

static size_t fill_length_octets(uint8* pool, size_t length, size_t capacity) {
    uint8 payload = 1U;
    
    if (length <= 127) {
        pool[capacity - payload] = (uint8)(length);
    } else {
        do {
            pool[capacity - payload] = (uint8)(length & 0xFF);
            length >>= 8U;
            payload++;
        } while (length > 0U);

        pool[capacity - payload] = 0b10000000 | (payload - 1);
    }

    return payload;
}

/*************************************************************************************************/
octets WarGrey::DTPM::asn_length_to_octets(size_t length) {
    uint8 pool[10];
    size_t capacity = sizeof(pool) / sizeof(uint8);
    size_t payload = fill_length_octets(pool, length, capacity);

    return octets(pool + (capacity - payload), payload);
}

size_t WarGrey::DTPM::asn_octets_to_length(octets& blength, size_t* offset) {
    size_t idx = ((offset == nullptr) ? 0 : (*offset));
    size_t length = blength[idx];

    if (length > 0b10000000) {
        size_t size = length & 0b01111111;

        length = 0;

        for (size_t i = 1; idx <= size; idx++) {
            length = (length << 8U) | blength[idx + i];
        }

        if (offset != nullptr) {
            (*offset) += (size + 1);
        }
    } else if (offset != nullptr) {
        (*offset) += 1;
    }

    return length;
}

/*************************************************************************************************/
bool WarGrey::DTPM::asn_primitive_predicate(ASNPrimitive type, octets& content, size_t offset) {
    return (asn_primitive_identifier(type) == content[offset]);
}

octets WarGrey::DTPM::asn_octets_box(uint8 tag, octets& content) {
    uint8 pool[10];
    size_t capacity = sizeof(pool) / sizeof(uint8);
    size_t payload = fill_length_octets(pool, content.size(), capacity);

    pool[capacity - payload] = tag;
    payload++;

    return octets(pool + (capacity - payload), payload).append(content);
}

size_t WarGrey::DTPM::asn_octets_unbox(WarGrey::DTPM::octets& basn, size_t* offset) {
    size_t content_idx = ((offset == nullptr) ? 0 : (*offset)) + 1U;
    size_t size = asn_octets_to_length(basn, &content_idx);

    if (offset != nullptr) {
        (*offset) = content_idx + size;
    }

    return size;
}
