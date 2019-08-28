/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PROFILE"
 * 	found in "ShareProfile.asn"
 * 	`asn1c -fcompound-names`
 */

#include "BootstrapResponse.h"

static asn_TYPE_member_t asn_MBR_BootstrapResponse_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct BootstrapResponse, result),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"result"
		},
};
static const ber_tlv_tag_t asn_DEF_BootstrapResponse_tags_1[] = {
	(ASN_TAG_CLASS_PRIVATE | (127 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_BootstrapResponse_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* result */
};
static asn_SEQUENCE_specifics_t asn_SPC_BootstrapResponse_specs_1 = {
	sizeof(struct BootstrapResponse),
	offsetof(struct BootstrapResponse, _asn_ctx),
	asn_MAP_BootstrapResponse_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_BootstrapResponse = {
	"BootstrapResponse",
	"BootstrapResponse",
	&asn_OP_SEQUENCE,
	asn_DEF_BootstrapResponse_tags_1,
	sizeof(asn_DEF_BootstrapResponse_tags_1)
		/sizeof(asn_DEF_BootstrapResponse_tags_1[0]) - 1, /* 1 */
	asn_DEF_BootstrapResponse_tags_1,	/* Same as above */
	sizeof(asn_DEF_BootstrapResponse_tags_1)
		/sizeof(asn_DEF_BootstrapResponse_tags_1[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_BootstrapResponse_1,
	1,	/* Elements count */
	&asn_SPC_BootstrapResponse_specs_1	/* Additional specs */
};

