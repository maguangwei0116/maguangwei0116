/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PEDefinitions"
 * 	found in "PEDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "ApplicationLoadPackage.h"

asn_TYPE_member_t asn_MBR_ApplicationLoadPackage_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ApplicationLoadPackage, loadPackageAID),
		(ASN_TAG_CLASS_APPLICATION | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ApplicationIdentifier,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"loadPackageAID"
		},
	{ ATF_POINTER, 5, offsetof(struct ApplicationLoadPackage, securityDomainAID),
		(ASN_TAG_CLASS_APPLICATION | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ApplicationIdentifier,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"securityDomainAID"
		},
	{ ATF_POINTER, 4, offsetof(struct ApplicationLoadPackage, nonVolatileCodeLimitC6),
		(ASN_TAG_CLASS_PRIVATE | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"nonVolatileCodeLimitC6"
		},
	{ ATF_POINTER, 3, offsetof(struct ApplicationLoadPackage, volatileDataLimitC7),
		(ASN_TAG_CLASS_PRIVATE | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"volatileDataLimitC7"
		},
	{ ATF_POINTER, 2, offsetof(struct ApplicationLoadPackage, nonVolatileDataLimitC8),
		(ASN_TAG_CLASS_PRIVATE | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"nonVolatileDataLimitC8"
		},
	{ ATF_POINTER, 1, offsetof(struct ApplicationLoadPackage, hashValue),
		(ASN_TAG_CLASS_PRIVATE | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"hashValue"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ApplicationLoadPackage, loadBlockObject),
		(ASN_TAG_CLASS_PRIVATE | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"loadBlockObject"
		},
};
static const int asn_MAP_ApplicationLoadPackage_oms_1[] = { 1, 2, 3, 4, 5 };
static const ber_tlv_tag_t asn_DEF_ApplicationLoadPackage_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ApplicationLoadPackage_tag2el_1[] = {
    { (ASN_TAG_CLASS_APPLICATION | (15 << 2)), 0, 0, 1 }, /* loadPackageAID */
    { (ASN_TAG_CLASS_APPLICATION | (15 << 2)), 1, -1, 0 }, /* securityDomainAID */
    { (ASN_TAG_CLASS_PRIVATE | (1 << 2)), 5, 0, 0 }, /* hashValue */
    { (ASN_TAG_CLASS_PRIVATE | (4 << 2)), 6, 0, 0 }, /* loadBlockObject */
    { (ASN_TAG_CLASS_PRIVATE | (6 << 2)), 2, 0, 0 }, /* nonVolatileCodeLimitC6 */
    { (ASN_TAG_CLASS_PRIVATE | (7 << 2)), 3, 0, 0 }, /* volatileDataLimitC7 */
    { (ASN_TAG_CLASS_PRIVATE | (8 << 2)), 4, 0, 0 } /* nonVolatileDataLimitC8 */
};
asn_SEQUENCE_specifics_t asn_SPC_ApplicationLoadPackage_specs_1 = {
	sizeof(struct ApplicationLoadPackage),
	offsetof(struct ApplicationLoadPackage, _asn_ctx),
	asn_MAP_ApplicationLoadPackage_tag2el_1,
	7,	/* Count of tags in the map */
	asn_MAP_ApplicationLoadPackage_oms_1,	/* Optional members */
	5, 0,	/* Root/Additions */
	7,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_ApplicationLoadPackage = {
	"ApplicationLoadPackage",
	"ApplicationLoadPackage",
	&asn_OP_SEQUENCE,
	asn_DEF_ApplicationLoadPackage_tags_1,
	sizeof(asn_DEF_ApplicationLoadPackage_tags_1)
		/sizeof(asn_DEF_ApplicationLoadPackage_tags_1[0]), /* 1 */
	asn_DEF_ApplicationLoadPackage_tags_1,	/* Same as above */
	sizeof(asn_DEF_ApplicationLoadPackage_tags_1)
		/sizeof(asn_DEF_ApplicationLoadPackage_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_ApplicationLoadPackage_1,
	7,	/* Elements count */
	&asn_SPC_ApplicationLoadPackage_specs_1	/* Additional specs */
};

