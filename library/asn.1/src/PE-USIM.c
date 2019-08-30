/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PEDefinitions"
 * 	found in "PEDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "PE-USIM.h"

asn_TYPE_member_t asn_MBR_PE_USIM_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PE_USIM, usim_header),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PEHeader,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"usim-header"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PE_USIM, templateID),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OBJECT_IDENTIFIER,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"templateID"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PE_USIM, adf_usim),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"adf-usim"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PE_USIM, ef_imsi),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-imsi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PE_USIM, ef_arr),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-arr"
		},
	{ ATF_POINTER, 3, offsetof(struct PE_USIM, ef_keys),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-keys"
		},
	{ ATF_POINTER, 2, offsetof(struct PE_USIM, ef_keysPS),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-keysPS"
		},
	{ ATF_POINTER, 1, offsetof(struct PE_USIM, ef_hpplmn),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-hpplmn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PE_USIM, ef_ust),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-ust"
		},
	{ ATF_POINTER, 4, offsetof(struct PE_USIM, ef_fdn),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-fdn"
		},
	{ ATF_POINTER, 3, offsetof(struct PE_USIM, ef_sms),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-sms"
		},
	{ ATF_POINTER, 2, offsetof(struct PE_USIM, ef_smsp),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-smsp"
		},
	{ ATF_POINTER, 1, offsetof(struct PE_USIM, ef_smss),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-smss"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PE_USIM, ef_spn),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-spn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PE_USIM, ef_est),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-est"
		},
	{ ATF_POINTER, 3, offsetof(struct PE_USIM, ef_start_hfn),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-start-hfn"
		},
	{ ATF_POINTER, 2, offsetof(struct PE_USIM, ef_threshold),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-threshold"
		},
	{ ATF_POINTER, 1, offsetof(struct PE_USIM, ef_psloci),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-psloci"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PE_USIM, ef_acc),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-acc"
		},
	{ ATF_POINTER, 3, offsetof(struct PE_USIM, ef_fplmn),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-fplmn"
		},
	{ ATF_POINTER, 2, offsetof(struct PE_USIM, ef_loci),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-loci"
		},
	{ ATF_POINTER, 1, offsetof(struct PE_USIM, ef_ad),
		(ASN_TAG_CLASS_CONTEXT | (21 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-ad"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PE_USIM, ef_ecc),
		(ASN_TAG_CLASS_CONTEXT | (22 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-ecc"
		},
	{ ATF_POINTER, 3, offsetof(struct PE_USIM, ef_netpar),
		(ASN_TAG_CLASS_CONTEXT | (23 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-netpar"
		},
	{ ATF_POINTER, 2, offsetof(struct PE_USIM, ef_epsloci),
		(ASN_TAG_CLASS_CONTEXT | (24 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-epsloci"
		},
	{ ATF_POINTER, 1, offsetof(struct PE_USIM, ef_epsnsc),
		(ASN_TAG_CLASS_CONTEXT | (25 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_File,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ef-epsnsc"
		},
};
static const int asn_MAP_PE_USIM_oms_1[] = { 5, 6, 7, 9, 10, 11, 12, 15, 16, 17, 19, 20, 21, 23, 24, 25 };
static const ber_tlv_tag_t asn_DEF_PE_USIM_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_PE_USIM_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* usim-header */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* templateID */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* adf-usim */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* ef-imsi */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* ef-arr */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* ef-keys */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* ef-keysPS */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* ef-hpplmn */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* ef-ust */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* ef-fdn */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* ef-sms */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* ef-smsp */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* ef-smss */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* ef-spn */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* ef-est */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* ef-start-hfn */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* ef-threshold */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* ef-psloci */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* ef-acc */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* ef-fplmn */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 }, /* ef-loci */
    { (ASN_TAG_CLASS_CONTEXT | (21 << 2)), 21, 0, 0 }, /* ef-ad */
    { (ASN_TAG_CLASS_CONTEXT | (22 << 2)), 22, 0, 0 }, /* ef-ecc */
    { (ASN_TAG_CLASS_CONTEXT | (23 << 2)), 23, 0, 0 }, /* ef-netpar */
    { (ASN_TAG_CLASS_CONTEXT | (24 << 2)), 24, 0, 0 }, /* ef-epsloci */
    { (ASN_TAG_CLASS_CONTEXT | (25 << 2)), 25, 0, 0 } /* ef-epsnsc */
};
asn_SEQUENCE_specifics_t asn_SPC_PE_USIM_specs_1 = {
	sizeof(struct PE_USIM),
	offsetof(struct PE_USIM, _asn_ctx),
	asn_MAP_PE_USIM_tag2el_1,
	26,	/* Count of tags in the map */
	asn_MAP_PE_USIM_oms_1,	/* Optional members */
	16, 0,	/* Root/Additions */
	26,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_PE_USIM = {
	"PE-USIM",
	"PE-USIM",
	&asn_OP_SEQUENCE,
	asn_DEF_PE_USIM_tags_1,
	sizeof(asn_DEF_PE_USIM_tags_1)
		/sizeof(asn_DEF_PE_USIM_tags_1[0]), /* 1 */
	asn_DEF_PE_USIM_tags_1,	/* Same as above */
	sizeof(asn_DEF_PE_USIM_tags_1)
		/sizeof(asn_DEF_PE_USIM_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_PE_USIM_1,
	26,	/* Elements count */
	&asn_SPC_PE_USIM_specs_1	/* Additional specs */
};

