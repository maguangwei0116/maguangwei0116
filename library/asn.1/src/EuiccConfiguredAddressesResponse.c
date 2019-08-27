/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "EuiccConfiguredAddressesResponse.h"

static asn_TYPE_member_t asn_MBR_EuiccConfiguredAddressesResponse_1[] = {
	{ ATF_POINTER, 1, offsetof(struct EuiccConfiguredAddressesResponse, defaultDpAddress),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_UTF8String,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"defaultDpAddress"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct EuiccConfiguredAddressesResponse, rootDsAddress),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_UTF8String,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"rootDsAddress"
		},
};
static const int asn_MAP_EuiccConfiguredAddressesResponse_oms_1[] = { 0 };
static const ber_tlv_tag_t asn_DEF_EuiccConfiguredAddressesResponse_tags_1[] = {
	(ASN_TAG_CLASS_CONTEXT | (60 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_EuiccConfiguredAddressesResponse_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* defaultDpAddress */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* rootDsAddress */
};
static asn_SEQUENCE_specifics_t asn_SPC_EuiccConfiguredAddressesResponse_specs_1 = {
	sizeof(struct EuiccConfiguredAddressesResponse),
	offsetof(struct EuiccConfiguredAddressesResponse, _asn_ctx),
	asn_MAP_EuiccConfiguredAddressesResponse_tag2el_1,
	2,	/* Count of tags in the map */
	asn_MAP_EuiccConfiguredAddressesResponse_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	2,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_EuiccConfiguredAddressesResponse = {
	"EuiccConfiguredAddressesResponse",
	"EuiccConfiguredAddressesResponse",
	&asn_OP_SEQUENCE,
	asn_DEF_EuiccConfiguredAddressesResponse_tags_1,
	sizeof(asn_DEF_EuiccConfiguredAddressesResponse_tags_1)
		/sizeof(asn_DEF_EuiccConfiguredAddressesResponse_tags_1[0]) - 1, /* 1 */
	asn_DEF_EuiccConfiguredAddressesResponse_tags_1,	/* Same as above */
	sizeof(asn_DEF_EuiccConfiguredAddressesResponse_tags_1)
		/sizeof(asn_DEF_EuiccConfiguredAddressesResponse_tags_1[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_EuiccConfiguredAddressesResponse_1,
	2,	/* Elements count */
	&asn_SPC_EuiccConfiguredAddressesResponse_specs_1	/* Additional specs */
};

