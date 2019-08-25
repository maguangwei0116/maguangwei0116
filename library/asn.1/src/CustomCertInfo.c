/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Credential"
 * 	found in "Credential.asn"
 * 	`asn1c -fcompound-names`
 */

#include "CustomCertInfo.h"

asn_TYPE_member_t asn_MBR_CustomCertInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CustomCertInfo, ciPkId),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ciPkId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CustomCertInfo, ciPk),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ciPk"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CustomCertInfo, euiccCrt),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"euiccCrt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CustomCertInfo, eumCrt),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"eumCrt"
		},
};
static const ber_tlv_tag_t asn_DEF_CustomCertInfo_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_CustomCertInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* ciPkId */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* ciPk */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* euiccCrt */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* eumCrt */
};
asn_SEQUENCE_specifics_t asn_SPC_CustomCertInfo_specs_1 = {
	sizeof(struct CustomCertInfo),
	offsetof(struct CustomCertInfo, _asn_ctx),
	asn_MAP_CustomCertInfo_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_CustomCertInfo = {
	"CustomCertInfo",
	"CustomCertInfo",
	&asn_OP_SEQUENCE,
	asn_DEF_CustomCertInfo_tags_1,
	sizeof(asn_DEF_CustomCertInfo_tags_1)
		/sizeof(asn_DEF_CustomCertInfo_tags_1[0]), /* 1 */
	asn_DEF_CustomCertInfo_tags_1,	/* Same as above */
	sizeof(asn_DEF_CustomCertInfo_tags_1)
		/sizeof(asn_DEF_CustomCertInfo_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_CustomCertInfo_1,
	4,	/* Elements count */
	&asn_SPC_CustomCertInfo_specs_1	/* Additional specs */
};

