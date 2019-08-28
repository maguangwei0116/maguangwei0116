/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PROFILE"
 * 	found in "ShareProfile.asn"
 * 	`asn1c -fcompound-names`
 */

#include "PreferredInfo.h"

static asn_TYPE_member_t asn_MBR_plmn_3[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_Plmn,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		""
		},
};
static const ber_tlv_tag_t asn_DEF_plmn_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_plmn_specs_3 = {
	sizeof(struct PreferredInfo__plmn),
	offsetof(struct PreferredInfo__plmn, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_plmn_3 = {
	"plmn",
	"plmn",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_plmn_tags_3,
	sizeof(asn_DEF_plmn_tags_3)
		/sizeof(asn_DEF_plmn_tags_3[0]) - 1, /* 1 */
	asn_DEF_plmn_tags_3,	/* Same as above */
	sizeof(asn_DEF_plmn_tags_3)
		/sizeof(asn_DEF_plmn_tags_3[0]), /* 2 */
	{ 0, 0, SEQUENCE_OF_constraint },
	asn_MBR_plmn_3,
	1,	/* Single element */
	&asn_SPC_plmn_specs_3	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_PreferredInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PreferredInfo, mcc),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"mcc"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PreferredInfo, plmn),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		0,
		&asn_DEF_plmn_3,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"plmn"
		},
};
static const ber_tlv_tag_t asn_DEF_PreferredInfo_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_PreferredInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* mcc */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* plmn */
};
asn_SEQUENCE_specifics_t asn_SPC_PreferredInfo_specs_1 = {
	sizeof(struct PreferredInfo),
	offsetof(struct PreferredInfo, _asn_ctx),
	asn_MAP_PreferredInfo_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_PreferredInfo = {
	"PreferredInfo",
	"PreferredInfo",
	&asn_OP_SEQUENCE,
	asn_DEF_PreferredInfo_tags_1,
	sizeof(asn_DEF_PreferredInfo_tags_1)
		/sizeof(asn_DEF_PreferredInfo_tags_1[0]), /* 1 */
	asn_DEF_PreferredInfo_tags_1,	/* Same as above */
	sizeof(asn_DEF_PreferredInfo_tags_1)
		/sizeof(asn_DEF_PreferredInfo_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_PreferredInfo_1,
	2,	/* Elements count */
	&asn_SPC_PreferredInfo_specs_1	/* Additional specs */
};

