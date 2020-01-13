#include "asn/identifier.hpp"

using namespace WarGrey::DTPM;

static uint8 identifiers[37];
static bool initialized = false;

static void initialize_identifiers() {
    if (!initialized) {
        size_t total = sizeof(identifiers) / sizeof(uint8);

        for (size_t idx = 0; idx < total; idx++) {
            identifiers[idx] = asn_identifier_octet((uint8)idx);
        }

        initialized = true;
    }
}

/*************************************************************************************************/
uint8 WarGrey::DTPM::asn_identifier_octet(uint8 tag, bool constructed, ASN1TagClass type) {
    uint8 type_octet = 0b00000000;

    switch (type) {
    case ASN1TagClass::Application:     type_octet = 0b01000000; break;
    case ASN1TagClass::ContextSpecific: type_octet = 0b10000000; break;
    case ASN1TagClass::Private:         type_octet = 0b11000000; break;
    }

    return type_octet | (constructed ? 0b00100000 : 0b00000000) | tag;
}

uint8 WarGrey::DTPM::asn_identifier_tag(uint8 octet) {
    return octet & 0b00011111;
}

ASN1TagClass WarGrey::DTPM::asn_identifier_class(uint8 octet) {
    ASN1TagClass tag = ASN1TagClass::Universal;

    switch (octet >> 6U) {
    case 0b01: tag = ASN1TagClass::Application; break;
    case 0b10: tag = ASN1TagClass::ContextSpecific; break;
    case 0b11: tag = ASN1TagClass::Private; break;
    }

    return tag;
}

bool WarGrey::DTPM::asn_identifier_constructed(uint8 octet) {
    return ((octet & 0b00100000) > 0U);
}

/*************************************************************************************************/
uint8 WarGrey::DTPM::asn_primitive_identifier(ASNPrimitive type) {
    size_t idx = 0;

    initialize_identifiers();

    switch (type) {
    case ASNPrimitive::Boolean:     idx = 0x01; break;
    case ASNPrimitive::Integer:     idx = 0x02; break;
    case ASNPrimitive::Null:        idx = 0x05; break;
    case ASNPrimitive::Real:        idx = 0x09; break;
    case ASNPrimitive::Enumerated:  idx = 0x0A; break;
    case ASNPrimitive::UTF8_String: idx = 0x0C; break;
    case ASNPrimitive::IA5_String:  idx = 0x16; break;
    }

    return identifiers[idx];
}
