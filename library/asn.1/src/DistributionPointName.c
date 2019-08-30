/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PKIX1Implicit88"
 * 	found in "PKIX1Implicit88.asn"
 * 	`asn1c -fcompound-names`
 */

#include "DistributionPointName.h"

static asn_oer_constraints_t asn_OER_type_DistributionPointName_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
asn_per_constraints_t asn_PER_type_DistributionPointName_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
asn_TYPE_member_t asn_MBR_DistributionPointName_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DistributionPointName, choice.fullName),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GeneralNames,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"fullName"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DistributionPointName, choice.nameRelativeToCRLIssuer),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RelativeDistinguishedName,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"nameRelativeToCRLIssuer"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_DistributionPointName_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* fullName */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* nameRelativeToCRLIssuer */
};
asn_CHOICE_specifics_t asn_SPC_DistributionPointName_specs_1 = {
	sizeof(struct DistributionPointName),
	offsetof(struct DistributionPointName, _asn_ctx),
	offsetof(struct DistributionPointName, present),
	sizeof(((struct DistributionPointName *)0)->present),
	asn_MAP_DistributionPointName_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_DistributionPointName = {
	"DistributionPointName",
	"DistributionPointName",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ &asn_OER_type_DistributionPointName_constr_1, &asn_PER_type_DistributionPointName_constr_1, CHOICE_constraint },
	asn_MBR_DistributionPointName_1,
	2,	/* Elements count */
	&asn_SPC_DistributionPointName_specs_1	/* Additional specs */
};

