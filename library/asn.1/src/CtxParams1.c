/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "CtxParams1.h"

static asn_oer_constraints_t asn_OER_type_CtxParams1_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
asn_per_constraints_t asn_PER_type_CtxParams1_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  0,  0,  0,  0 }	/* (0..0,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
asn_TYPE_member_t asn_MBR_CtxParams1_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CtxParams1, choice.ctxParamsForCommonAuthentication),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CtxParamsForCommonAuthentication,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ctxParamsForCommonAuthentication"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_CtxParams1_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* ctxParamsForCommonAuthentication */
};
asn_CHOICE_specifics_t asn_SPC_CtxParams1_specs_1 = {
	sizeof(struct CtxParams1),
	offsetof(struct CtxParams1, _asn_ctx),
	offsetof(struct CtxParams1, present),
	sizeof(((struct CtxParams1 *)0)->present),
	asn_MAP_CtxParams1_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0,
	1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_CtxParams1 = {
	"CtxParams1",
	"CtxParams1",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ &asn_OER_type_CtxParams1_constr_1, &asn_PER_type_CtxParams1_constr_1, CHOICE_constraint },
	asn_MBR_CtxParams1_1,
	1,	/* Elements count */
	&asn_SPC_CtxParams1_specs_1	/* Additional specs */
};

