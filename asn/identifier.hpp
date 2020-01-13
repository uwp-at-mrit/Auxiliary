#pragma once

namespace WarGrey::DTPM {
    private enum class ASNPrimitive {
        Boolean, Integer, /* BitString, OctetString, */ Null,
        /* ObjectIdentifier, ObjectDescriptor, External, */
        Real, Enumerated,
        UTF8_String, IA5_String
    };

    private enum class ASN1TagClass { Universal, Application, ContextSpecific, Private };

    uint8 asn_identifier_octet(uint8 tag, bool constructed = false, WarGrey::DTPM::ASN1TagClass type = ASN1TagClass::Universal);

    uint8 asn_identifier_tag(uint8 octet);
    WarGrey::DTPM::ASN1TagClass asn_identifier_class(uint8 octet);
    bool asn_identifier_constructed(uint8 octet);

    uint8 asn_primitive_identifier(WarGrey::DTPM::ASNPrimitive type);
}
