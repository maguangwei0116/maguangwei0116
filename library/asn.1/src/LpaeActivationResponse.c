/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#include "LpaeActivationResponse.h"

static asn_TYPE_member_t asn_MBR_LpaeActivationResponse_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct LpaeActivationResponse, lpaeActivationResult),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"lpaeActivationResult"
		},
};
static const ber_tlv_tag_t asn_DEF_LpaeActivationResponse_tags_1[] = {
	(ASN_TAG_CLASS_CONTEXT | (66 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_LpaeActivationResponse_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* lpaeActivationResult */
};
static asn_SEQUENCE_specifics_t asn_SPC_LpaeActivationResponse_specs_1 = {
	sizeof(struct LpaeActivationResponse),
	offsetof(struct LpaeActivationResponse, _asn_ctx),
	asn_MAP_LpaeActivationResponse_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_LpaeActivationResponse = {
	"LpaeActivationResponse",
	"LpaeActivationResponse",
	&asn_OP_SEQUENCE,
	asn_DEF_LpaeActivationResponse_tags_1,
	sizeof(asn_DEF_LpaeActivationResponse_tags_1)
		/sizeof(asn_DEF_LpaeActivationResponse_tags_1[0]) - 1, /* 1 */
	asn_DEF_LpaeActivationResponse_tags_1,	/* Same as above */
	sizeof(asn_DEF_LpaeActivationResponse_tags_1)
		/sizeof(asn_DEF_LpaeActivationResponse_tags_1[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_LpaeActivationResponse_1,
	1,	/* Elements count */
	&asn_SPC_LpaeActivationResponse_specs_1	/* Additional specs */
};

