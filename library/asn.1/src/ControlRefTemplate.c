/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "ControlRefTemplate.h"

asn_TYPE_member_t asn_MBR_ControlRefTemplate_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ControlRefTemplate, keyType),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Octet1,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"keyType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ControlRefTemplate, keyLen),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Octet1,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"keyLen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ControlRefTemplate, hostId),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OctetTo16,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"hostId"
		},
};
static const ber_tlv_tag_t asn_DEF_ControlRefTemplate_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ControlRefTemplate_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* keyType */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* keyLen */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 2, 0, 0 } /* hostId */
};
asn_SEQUENCE_specifics_t asn_SPC_ControlRefTemplate_specs_1 = {
	sizeof(struct ControlRefTemplate),
	offsetof(struct ControlRefTemplate, _asn_ctx),
	asn_MAP_ControlRefTemplate_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	3,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_ControlRefTemplate = {
	"ControlRefTemplate",
	"ControlRefTemplate",
	&asn_OP_SEQUENCE,
	asn_DEF_ControlRefTemplate_tags_1,
	sizeof(asn_DEF_ControlRefTemplate_tags_1)
		/sizeof(asn_DEF_ControlRefTemplate_tags_1[0]), /* 1 */
	asn_DEF_ControlRefTemplate_tags_1,	/* Same as above */
	sizeof(asn_DEF_ControlRefTemplate_tags_1)
		/sizeof(asn_DEF_ControlRefTemplate_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_ControlRefTemplate_1,
	3,	/* Elements count */
	&asn_SPC_ControlRefTemplate_specs_1	/* Additional specs */
};

