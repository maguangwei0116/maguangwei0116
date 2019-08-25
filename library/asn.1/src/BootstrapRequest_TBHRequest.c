/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "BootstrapRequest"
 * 	found in "ShareProfile.asn"
 * 	`asn1c -fcompound-names`
 */

#include "BootstrapRequest_TBHRequest.h"

asn_TYPE_member_t asn_MBR_BootstrapRequest_TBHRequest_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct BootstrapRequest_TBHRequest, iccid),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"iccid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct BootstrapRequest_TBHRequest, imsi),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"imsi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct BootstrapRequest_TBHRequest, key),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"key"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct BootstrapRequest_TBHRequest, opc),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"opc"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct BootstrapRequest_TBHRequest, rotation),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"rotation"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct BootstrapRequest_TBHRequest, xoring),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"xoring"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct BootstrapRequest_TBHRequest, sqnFlag),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"sqnFlag"
		},
	{ ATF_POINTER, 5, offsetof(struct BootstrapRequest_TBHRequest, rplmn),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"rplmn"
		},
	{ ATF_POINTER, 4, offsetof(struct BootstrapRequest_TBHRequest, fplmn),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"fplmn"
		},
	{ ATF_POINTER, 3, offsetof(struct BootstrapRequest_TBHRequest, hplmn),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"hplmn"
		},
	{ ATF_POINTER, 2, offsetof(struct BootstrapRequest_TBHRequest, ehplmn),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ehplmn"
		},
	{ ATF_POINTER, 1, offsetof(struct BootstrapRequest_TBHRequest, oplmn),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"oplmn"
		},
};
static const int asn_MAP_BootstrapRequest_TBHRequest_oms_1[] = { 7, 8, 9, 10, 11 };
static const ber_tlv_tag_t asn_DEF_BootstrapRequest_TBHRequest_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_BootstrapRequest_TBHRequest_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* iccid */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* imsi */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* key */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* opc */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* rotation */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* xoring */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* sqnFlag */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* rplmn */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* fplmn */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* hplmn */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* ehplmn */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 } /* oplmn */
};
asn_SEQUENCE_specifics_t asn_SPC_BootstrapRequest_TBHRequest_specs_1 = {
	sizeof(struct BootstrapRequest_TBHRequest),
	offsetof(struct BootstrapRequest_TBHRequest, _asn_ctx),
	asn_MAP_BootstrapRequest_TBHRequest_tag2el_1,
	12,	/* Count of tags in the map */
	asn_MAP_BootstrapRequest_TBHRequest_oms_1,	/* Optional members */
	5, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_BootstrapRequest_TBHRequest = {
	"TBHRequest",
	"TBHRequest",
	&asn_OP_SEQUENCE,
	asn_DEF_BootstrapRequest_TBHRequest_tags_1,
	sizeof(asn_DEF_BootstrapRequest_TBHRequest_tags_1)
		/sizeof(asn_DEF_BootstrapRequest_TBHRequest_tags_1[0]), /* 1 */
	asn_DEF_BootstrapRequest_TBHRequest_tags_1,	/* Same as above */
	sizeof(asn_DEF_BootstrapRequest_TBHRequest_tags_1)
		/sizeof(asn_DEF_BootstrapRequest_TBHRequest_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_BootstrapRequest_TBHRequest_1,
	12,	/* Elements count */
	&asn_SPC_BootstrapRequest_TBHRequest_specs_1	/* Additional specs */
};

