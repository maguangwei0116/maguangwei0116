/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#include "DisableProfileRequest.h"

static asn_oer_constraints_t asn_OER_type_profileIdentifier_constr_2 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_profileIdentifier_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_profileIdentifier_2[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DisableProfileRequest__profileIdentifier, choice.isdpAid),
		(ASN_TAG_CLASS_APPLICATION | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OctetTo16,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"isdpAid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DisableProfileRequest__profileIdentifier, choice.iccid),
		(ASN_TAG_CLASS_APPLICATION | (26 << 2)),
		0,
		&asn_DEF_Iccid,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"iccid"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_profileIdentifier_tag2el_2[] = {
    { (ASN_TAG_CLASS_APPLICATION | (15 << 2)), 0, 0, 0 }, /* isdpAid */
    { (ASN_TAG_CLASS_APPLICATION | (26 << 2)), 1, 0, 0 } /* iccid */
};
static asn_CHOICE_specifics_t asn_SPC_profileIdentifier_specs_2 = {
	sizeof(struct DisableProfileRequest__profileIdentifier),
	offsetof(struct DisableProfileRequest__profileIdentifier, _asn_ctx),
	offsetof(struct DisableProfileRequest__profileIdentifier, present),
	sizeof(((struct DisableProfileRequest__profileIdentifier *)0)->present),
	asn_MAP_profileIdentifier_tag2el_2,
	2,	/* Count of tags in the map */
	0, 0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_profileIdentifier_2 = {
	"profileIdentifier",
	"profileIdentifier",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ &asn_OER_type_profileIdentifier_constr_2, &asn_PER_type_profileIdentifier_constr_2, CHOICE_constraint },
	asn_MBR_profileIdentifier_2,
	2,	/* Elements count */
	&asn_SPC_profileIdentifier_specs_2	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_DisableProfileRequest_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DisableProfileRequest, profileIdentifier),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_profileIdentifier_2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"profileIdentifier"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DisableProfileRequest, refreshFlag),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"refreshFlag"
		},
};
static const ber_tlv_tag_t asn_DEF_DisableProfileRequest_tags_1[] = {
	(ASN_TAG_CLASS_CONTEXT | (50 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_DisableProfileRequest_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* profileIdentifier */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* refreshFlag */
};
static asn_SEQUENCE_specifics_t asn_SPC_DisableProfileRequest_specs_1 = {
	sizeof(struct DisableProfileRequest),
	offsetof(struct DisableProfileRequest, _asn_ctx),
	asn_MAP_DisableProfileRequest_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_DisableProfileRequest = {
	"DisableProfileRequest",
	"DisableProfileRequest",
	&asn_OP_SEQUENCE,
	asn_DEF_DisableProfileRequest_tags_1,
	sizeof(asn_DEF_DisableProfileRequest_tags_1)
		/sizeof(asn_DEF_DisableProfileRequest_tags_1[0]) - 1, /* 1 */
	asn_DEF_DisableProfileRequest_tags_1,	/* Same as above */
	sizeof(asn_DEF_DisableProfileRequest_tags_1)
		/sizeof(asn_DEF_DisableProfileRequest_tags_1[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_DisableProfileRequest_1,
	2,	/* Elements count */
	&asn_SPC_DisableProfileRequest_specs_1	/* Additional specs */
};

