/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#include "ProfileInfoListResponse.h"

static asn_oer_constraints_t asn_OER_type_ProfileInfoListResponse_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_ProfileInfoListResponse_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_profileInfoListOk_2[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_PRIVATE | (3 << 2)),
		0,
		&asn_DEF_ProfileInfo,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		""
		},
};
static const ber_tlv_tag_t asn_DEF_profileInfoListOk_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_profileInfoListOk_specs_2 = {
	sizeof(struct ProfileInfoListResponse__profileInfoListOk),
	offsetof(struct ProfileInfoListResponse__profileInfoListOk, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_profileInfoListOk_2 = {
	"profileInfoListOk",
	"profileInfoListOk",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_profileInfoListOk_tags_2,
	sizeof(asn_DEF_profileInfoListOk_tags_2)
		/sizeof(asn_DEF_profileInfoListOk_tags_2[0]) - 1, /* 1 */
	asn_DEF_profileInfoListOk_tags_2,	/* Same as above */
	sizeof(asn_DEF_profileInfoListOk_tags_2)
		/sizeof(asn_DEF_profileInfoListOk_tags_2[0]), /* 2 */
	{ 0, 0, SEQUENCE_OF_constraint },
	asn_MBR_profileInfoListOk_2,
	1,	/* Single element */
	&asn_SPC_profileInfoListOk_specs_2	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_ProfileInfoListResponse_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ProfileInfoListResponse, choice.profileInfoListOk),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		0,
		&asn_DEF_profileInfoListOk_2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"profileInfoListOk"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ProfileInfoListResponse, choice.profileInfoListError),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ProfileInfoListError,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"profileInfoListError"
		},
};
static const ber_tlv_tag_t asn_DEF_ProfileInfoListResponse_tags_1[] = {
	(ASN_TAG_CLASS_CONTEXT | (45 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ProfileInfoListResponse_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* profileInfoListOk */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* profileInfoListError */
};
static asn_CHOICE_specifics_t asn_SPC_ProfileInfoListResponse_specs_1 = {
	sizeof(struct ProfileInfoListResponse),
	offsetof(struct ProfileInfoListResponse, _asn_ctx),
	offsetof(struct ProfileInfoListResponse, present),
	sizeof(((struct ProfileInfoListResponse *)0)->present),
	asn_MAP_ProfileInfoListResponse_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_ProfileInfoListResponse = {
	"ProfileInfoListResponse",
	"ProfileInfoListResponse",
	&asn_OP_CHOICE,
	asn_DEF_ProfileInfoListResponse_tags_1,
	sizeof(asn_DEF_ProfileInfoListResponse_tags_1)
		/sizeof(asn_DEF_ProfileInfoListResponse_tags_1[0]), /* 1 */
	asn_DEF_ProfileInfoListResponse_tags_1,	/* Same as above */
	sizeof(asn_DEF_ProfileInfoListResponse_tags_1)
		/sizeof(asn_DEF_ProfileInfoListResponse_tags_1[0]), /* 1 */
	{ &asn_OER_type_ProfileInfoListResponse_constr_1, &asn_PER_type_ProfileInfoListResponse_constr_1, CHOICE_constraint },
	asn_MBR_ProfileInfoListResponse_1,
	2,	/* Elements count */
	&asn_SPC_ProfileInfoListResponse_specs_1	/* Additional specs */
};

