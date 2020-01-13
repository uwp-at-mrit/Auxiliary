#include "asn/base.hpp"

#include "datum/box.hpp"
#include "datum/flonum.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

/*************************************************************************************************/
static inline size_t fill_ufixnum_octets(uint8* pool, unsigned long long n, size_t capacity, size_t payload) {
    do {
        payload++;
        pool[capacity - payload] = (uint8)(n & 0xFF);
        n >>= 8U;
    } while (n > 0U);

    return payload;
}

static inline size_t fill_fixnum_octets(uint8* pool, long long n, size_t capacity, size_t payload) {
    if (n >= 0) {
        payload = fill_ufixnum_octets(pool, n, capacity, payload);

        if (pool[capacity - payload] >= 0b10000000) {
            pool[capacity - (++payload)] = 0;
        }
    } else {
        do {
            payload++;
            pool[capacity - payload] = (uint8)(n & 0xFF);
            n >>= 8U;
        } while (n < -1);
    }

    return payload;
}

static inline size_t fill_length_octets(uint8* pool, size_t length, size_t capacity) {
    size_t payload = 1U;
    
    if (length <= 127) {
        pool[capacity - payload] = (uint8)(length);
    } else {
        payload = fill_ufixnum_octets(pool, length, capacity, payload - 1);
        pool[capacity - (++payload)] = (uint8)(0b10000000 | payload);
    }

    return payload;
}

static inline void substitude_trailing_zero(long long* E, long long* N, uint8 delta, uint64 mask, uint8 rshift) {
    while (((*N) & mask) == 0) {
        (*E) += delta;
        (*N) >>= rshift;
    }
}

static void fill_real_binary(double real, double base, long long* E, long long* N) {
    double r = real;

    (*E) = 0;

    // TODO: implement Racket `integer?`
    while (flfloor(r) != r) {
        r *= base;
        (*E) -= 1U;
    }

    (*N) = (long long)flfloor(r);

    if (base == 16.0) {
        substitude_trailing_zero(E, N, 2, 0xFF, -8);
    } else if (base == 2.0) {
        substitude_trailing_zero(E, N, 8, 0xFF, -8);
    } else {
        substitude_trailing_zero(E, N, 1, 0b111, -3);
    }
}

template<typename N>
static inline void fill_integer_from_bytes(N* n, octets& pool, size_t start, size_t end, bool check_sign = false) {
    if (check_sign) {
        (*n) = ((pool[start] >= 0b10000000) ? -1 : 0);
    } else {
        (*n) = 0;
    }

    for (size_t idx = start; idx < end; idx++) {
        (*n) = ((*n) << 8U) | pool[idx];
    }
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

        fill_integer_from_bytes(&length, blength, idx + 1, idx + size + 1, false);

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

octets WarGrey::DTPM::asn_octets_box(uint8 tag, octets& content, size_t size) {
    uint8 pool[10];
    size_t capacity = sizeof(pool) / sizeof(uint8);
    size_t payload = fill_length_octets(pool, size, capacity);

    pool[capacity - (++payload)] = tag;

    return octets(pool + (capacity - payload), payload).append(content);
}

size_t WarGrey::DTPM::asn_octets_unbox(WarGrey::DTPM::octets& basn, size_t* offset) {
    size_t content_idx = ((offset == nullptr) ? 0 : (*offset)) + 1U;
    size_t size = asn_octets_to_length(basn, &content_idx);

    SET_BOX(offset, (content_idx + size));

    return size;
}

/*************************************************************************************************/
octets WarGrey::DTPM::asn_boolean_to_octets(bool b) {
    uint8 pool[3];
   
    pool[0] = asn_primitive_identifier(ASNPrimitive::Boolean);
    pool[1] = 0x01;
    pool[2] = (b ? 0xFF : 0x00);

    return octets(pool, sizeof(pool)/sizeof(uint8));
}

bool WarGrey::DTPM::asn_octets_to_boolean(octets& bnat, size_t* offset0) {
    size_t offset = ((offset0 == nullptr) ? 0 : (*offset0));
    size_t size = asn_octets_unbox(bnat, &offset);
    
    SET_BOX(offset0, offset);

    return (bnat[offset - size] > 0x00);
}

octets WarGrey::DTPM::asn_fixnum_to_octets(long long fixnum) {
    uint8 pool[12];
    size_t capacity = sizeof(pool) / sizeof(uint8);
    size_t payload = fill_fixnum_octets(pool, fixnum, capacity, 0U);

    pool[capacity - (++payload)] = (uint8)payload;
    pool[capacity - (++payload)] = asn_primitive_identifier(ASNPrimitive::Integer);

    return octets(pool + (capacity - payload), payload);
}

long long WarGrey::DTPM::asn_octets_to_fixnum(octets& bfixnum, size_t* offset0) {
    size_t offset = ((offset0 == nullptr) ? 0 : (*offset0));
    size_t size = asn_octets_unbox(bfixnum, &offset);
    long long integer = 0;

    fill_integer_from_bytes(&integer, bfixnum, offset - size, offset, true);

    SET_BOX(offset0, offset);

    return integer;
}

octets WarGrey::DTPM::asn_natural_to_octets(Natural& nat) {
    octets payload = nat.to_bytes();
    size_t size = nat.length();
    
    if (payload[0] >= 0b10000000) {
        payload.insert(0, 1, '\x00');
        size += 1;
    }

    return asn_octets_box(asn_primitive_identifier(ASNPrimitive::Integer), payload, size);
}

Natural WarGrey::DTPM::asn_octets_to_natural(octets& bnat, size_t* offset0) {
    size_t offset = ((offset0 == nullptr) ? 0 : (*offset0));
    size_t size = asn_octets_unbox(bnat, &offset);
    Natural nat(bnat, offset - size, offset);

    SET_BOX(offset0, offset);

    return nat;
}

octets WarGrey::DTPM::asn_null_to_octets(std::nullptr_t placeholder) {
    uint8 pool[2];

    pool[0] = asn_primitive_identifier(ASNPrimitive::Null);
    pool[1] = 0x00;

    return octets(pool, sizeof(pool) / sizeof(uint8));
}

std::nullptr_t WarGrey::DTPM::asn_octets_to_null(octets& bnat, size_t* offset0) {
    size_t offset = ((offset0 == nullptr) ? 0 : (*offset0));
    size_t size = asn_octets_unbox(bnat, &offset);

    SET_BOX(offset0, offset);

    return nullptr;
}

octets WarGrey::DTPM::asn_real_to_octets(double real) {
    uint8 pool[64];
    size_t capacity = sizeof(pool) / sizeof(uint8);
    size_t payload = 0U;
    uint8 base = 2;

    // TODO: implememnt base 10 representation

    if (!flisfinite(real)) {
        payload = 1U;
        
        if (real > 0.0) {
            pool[capacity - payload] = 0x40;
        } else if (real < 0.0) {
            pool[capacity - payload] = 0x41;
        } else {
            pool[capacity - payload] = 0x42;
        }
    } else if (real == 0.0) {
        // WARNING: 0.0 is +0.0, hence 1.0
        if (flsign(real) == -1.0) {
            payload = 1U;
            pool[capacity - payload] = 0x43;
        }
    } else {
        uint8 infoctet = ((real > 0.0) ? 0b10000000 : 0b11000000);
        size_t info_size;
        long long E, N;
        
        fill_real_binary(flabs(real), double(base), &E, &N);
        info_size = fill_fixnum_octets(pool, N, capacity, payload);
        payload = fill_fixnum_octets(pool, E, capacity, info_size);
        info_size = payload - info_size;

        switch (base) {
        case 8:  infoctet ^= 0b00010000U; break;
        case 16: infoctet ^= 0b00100000U; break;
        }

        switch (info_size) {
        case 2:  infoctet ^= 0b00000001U; break;
        case 3:  infoctet ^= 0b00000010U; break;
        }

        if (info_size >= 4) {
            pool[capacity - (++payload)] = (uint8)info_size;
            infoctet ^= 0b00000011U;
        }

        pool[capacity - (++payload)] = (uint8)infoctet;
    }

    pool[capacity - (++payload)] = (uint8)payload;
    pool[capacity - (++payload)] = asn_primitive_identifier(ASNPrimitive::Real);

    return octets(pool + (capacity - payload), payload);
}

double WarGrey::DTPM::asn_octets_to_real(octets& breal, size_t* offset0) {
    size_t offset = ((offset0 == nullptr) ? 0 : (*offset0));
    size_t size = asn_octets_unbox(breal, &offset);
    double real = flnan;

    if (size > 0) {
        uint8 infoctet = breal[offset - size];

        if ((infoctet & (0b1 << 7)) > 0) {
            double sign = ((infoctet & (0b1 << 6)) > 0) ? -1.0 : 1.0;
            double base = flnan;
            uint8 N_lshift = ((infoctet & 0b1100) >> 2);
            size_t E_start = offset - size + 1;
            size_t E_end = offset;

            switch ((infoctet & 0b110000) >> 4) {
            case 0b00: base = 2.0; break;
            case 0b01: base = 8.0; break;
            case 0b10: base = 16.0; break;
            }

            switch (infoctet & 0b11) {
            case 0b00: E_end = E_start + 1; break;
            case 0b01: E_end = E_start + 2; break;
            case 0b11: E_end = E_start + 3; break;
            default: E_start += 1; E_end = E_start + breal[E_start - 1];
            }

            if (E_end < offset) {
                long long E, N;

                fill_integer_from_bytes(&E, breal, E_start, E_end, true);
                fill_integer_from_bytes(&N, breal, E_end, offset, true);
                real = sign * double(N << N_lshift) * flexpt(base, double(E));
            } else if (E_end == offset) {
                real = 0.0;
            }
        } else {
            switch (infoctet) {
            case 0b01000000: real = +infinity; break;
            case 0b01000001: real = -infinity; break;
            case 0b01000010: real = flnan; break;
            case 0b01000011: real = -0.0; break;
            }
        }
    } else if (size == 0) {
        real = 0.0;
    }

    SET_BOX(offset0, offset);

    return real;
}

octets WarGrey::DTPM::asn_ia5_to_octets(std::string& str) {
    size_t size = str.length();
    octets payload((uint8*)str.c_str(), 0, size);
   
    return asn_octets_box(asn_primitive_identifier(ASNPrimitive::IA5), payload, size);
}

std::string WarGrey::DTPM::asn_octets_to_ia5(octets& bnat, size_t* offset0) {
    size_t offset = ((offset0 == nullptr) ? 0 : (*offset0));
    size_t size = asn_octets_unbox(bnat, &offset);
    Natural nat(bnat, offset - size, offset);

    SET_BOX(offset0, offset);

    return std::string((char*)bnat.c_str(), offset - size, size);
}
