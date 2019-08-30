/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "EUICCInfo1.h"

static asn_TYPE_member_t asn_MBR_euiccCiPKIdListForVerification_3[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_SubjectKeyIdentifier,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		""
		},
};
static const ber_tlv_tag_t asn_DEF_euiccCiPKIdListForVerification_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_euiccCiPKIdListForVerification_specs_3 = {
	sizeof(struct EUICCInfo1__euiccCiPKIdListForVerification),
	offsetof(struct EUICCInfo1__euiccCiPKIdListForVerification, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_euiccCiPKIdListForVerification_3 = {
	"euiccCiPKIdListForVerification",
	"euiccCiPKIdListForVerification",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_euiccCiPKIdListForVerification_tags_3,
	sizeof(asn_DEF_euiccCiPKIdListForVerification_tags_3)
		/sizeof(asn_DEF_euiccCiPKIdListForVerification_tags_3[0]) - 1, /* 1 */
	asn_DEF_euiccCiPKIdListForVerification_tags_3,	/* Same as above */
	sizeof(asn_DEF_euiccCiPKIdListForVerification_tags_3)
		/sizeof(asn_DEF_euiccCiPKIdListForVerification_tags_3[0]), /* 2 */
	{ 0, 0, SEQUENCE_OF_constraint },
	asn_MBR_euiccCiPKIdListForVerification_3,
	1,	/* Single element */
	&asn_SPC_euiccCiPKIdListForVerification_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_euiccCiPKIdListForSigning_5[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_SubjectKeyIdentifier,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		""
		},
};
static const ber_tlv_tag_t asn_DEF_euiccCiPKIdListForSigning_tags_5[] = {
	(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_euiccCiPKIdListForSigning_specs_5 = {
	sizeof(struct EUICCInfo1__euiccCiPKIdListForSigning),
	offsetof(struct EUICCInfo1__euiccCiPKIdListForSigning, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_euiccCiPKIdListForSigning_5 = {
	"euiccCiPKIdListForSigning",
	"euiccCiPKIdListForSigning",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_euiccCiPKIdListForSigning_tags_5,
	sizeof(asn_DEF_euiccCiPKIdListForSigning_tags_5)
		/sizeof(asn_DEF_euiccCiPKIdListForSigning_tags_5[0]) - 1, /* 1 */
	asn_DEF_euiccCiPKIdListForSigning_tags_5,	/* Same as above */
	sizeof(asn_DEF_euiccCiPKIdListForSigning_tags_5)
		/sizeof(asn_DEF_euiccCiPKIdListForSigning_tags_5[0]), /* 2 */
	{ 0, 0, SEQUENCE_OF_constraint },
	asn_MBR_euiccCiPKIdListForSigning_5,
	1,	/* Single element */
	&asn_SPC_euiccCiPKIdListForSigning_specs_5	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_EUICCInfo1_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct EUICCInfo1, svn),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_VersionType,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"svn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct EUICCInfo1, euiccCiPKIdListForVerification),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		0,
		&asn_DEF_euiccCiPKIdListForVerification_3,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"euiccCiPKIdListForVerification"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct EUICCInfo1, euiccCiPKIdListForSigning),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		0,
		&asn_DEF_euiccCiPKIdListForSigning_5,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"euiccCiPKIdListForSigning"
		},
};
static const ber_tlv_tag_t asn_DEF_EUICCInfo1_tags_1[] = {
	(ASN_TAG_CLASS_CONTEXT | (32 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_EUICCInfo1_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 0, 0, 0 }, /* svn */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 1, 0, 0 }, /* euiccCiPKIdListForVerification */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 2, 0, 0 } /* euiccCiPKIdListForSigning */
};
asn_SEQUENCE_specifics_t asn_SPC_EUICCInfo1_specs_1 = {
	sizeof(struct EUICCInfo1),
	offsetof(struct EUICCInfo1, _asn_ctx),
	asn_MAP_EUICCInfo1_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	3,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_EUICCInfo1 = {
	"EUICCInfo1",
	"EUICCInfo1",
	&asn_OP_SEQUENCE,
	asn_DEF_EUICCInfo1_tags_1,
	sizeof(asn_DEF_EUICCInfo1_tags_1)
		/sizeof(asn_DEF_EUICCInfo1_tags_1[0]) - 1, /* 1 */
	asn_DEF_EUICCInfo1_tags_1,	/* Same as above */
	sizeof(asn_DEF_EUICCInfo1_tags_1)
		/sizeof(asn_DEF_EUICCInfo1_tags_1[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_EUICCInfo1_1,
	3,	/* Elements count */
	&asn_SPC_EUICCInfo1_specs_1	/* Additional specs */
};

