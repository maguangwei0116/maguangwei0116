/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Credential"
 * 	found in "Credential.asn"
 * 	`asn1c -fcompound-names`
 */

#include "SignResponse.h"

static asn_oer_constraints_t asn_OER_type_SignResponse_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_SignResponse_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_SignResponse_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SignResponse, choice.signSuccessResult),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"signSuccessResult"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SignResponse, choice.signErrorResult),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"signErrorResult"
		},
};
static const ber_tlv_tag_t asn_DEF_SignResponse_tags_1[] = {
	(ASN_TAG_CLASS_PRIVATE | (33 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_SignResponse_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* signSuccessResult */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* signErrorResult */
};
static asn_CHOICE_specifics_t asn_SPC_SignResponse_specs_1 = {
	sizeof(struct SignResponse),
	offsetof(struct SignResponse, _asn_ctx),
	offsetof(struct SignResponse, present),
	sizeof(((struct SignResponse *)0)->present),
	asn_MAP_SignResponse_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_SignResponse = {
	"SignResponse",
	"SignResponse",
	&asn_OP_CHOICE,
	asn_DEF_SignResponse_tags_1,
	sizeof(asn_DEF_SignResponse_tags_1)
		/sizeof(asn_DEF_SignResponse_tags_1[0]), /* 1 */
	asn_DEF_SignResponse_tags_1,	/* Same as above */
	sizeof(asn_DEF_SignResponse_tags_1)
		/sizeof(asn_DEF_SignResponse_tags_1[0]), /* 1 */
	{ &asn_OER_type_SignResponse_constr_1, &asn_PER_type_SignResponse_constr_1, CHOICE_constraint },
	asn_MBR_SignResponse_1,
	2,	/* Elements count */
	&asn_SPC_SignResponse_specs_1	/* Additional specs */
};

