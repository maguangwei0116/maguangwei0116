/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PKIX1Implicit88"
 * 	found in "PKIX1Implicit88.asn"
 * 	`asn1c -fcompound-names`
 */

#include "BasicConstraints.h"

static int
pathLenConstraint_3_constraint(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	unsigned long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const unsigned long *)sptr;
	
	if(1 /* No applicable constraints whatsoever */) {
		(void)value; /* Unused variable */
		/* Nothing is here. See below */
	}
	
	return td->encoding_constraints.general_constraints(td, sptr, ctfailcb, app_key);
}

/*
 * This type is implemented using NativeInteger,
 * so here we adjust the DEF accordingly.
 */
static int
memb_pathLenConstraint_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	unsigned long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const unsigned long *)sptr;
	
	if(1 /* No applicable constraints whatsoever */) {
		(void)value; /* Unused variable */
		/* Nothing is here. See below */
	}
	
	return td->encoding_constraints.general_constraints(td, sptr, ctfailcb, app_key);
}

static asn_oer_constraints_t asn_OER_type_pathLenConstraint_constr_3 CC_NOTUSED = {
	{ 0, 1 }	/* (0..MAX) */,
	-1};
static asn_per_constraints_t asn_PER_type_pathLenConstraint_constr_3 CC_NOTUSED = {
	{ APC_SEMI_CONSTRAINED,	-1, -1,  0,  0 }	/* (0..MAX) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_pathLenConstraint_constr_3 CC_NOTUSED = {
	{ 0, 1 }	/* (0..MAX) */,
	-1};
static asn_per_constraints_t asn_PER_memb_pathLenConstraint_constr_3 CC_NOTUSED = {
	{ APC_SEMI_CONSTRAINED,	-1, -1,  0,  0 }	/* (0..MAX) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static int asn_DFL_2_cmp_0(const void *sptr) {
	const BOOLEAN_t *st = sptr;
	
	if(!st) {
		return -1; /* No value is not a default value */
	}
	
	/* Test default value 0 */
	return (*st != 0);
}
static int asn_DFL_2_set_0(void **sptr) {
	BOOLEAN_t *st = *sptr;
	
	if(!st) {
		st = (*sptr = CALLOC(1, sizeof(*st)));
		if(!st) return -1;
	}
	
	/* Install default value 0 */
	*st = 0;
	return 0;
}
static const asn_INTEGER_specifics_t asn_SPC_pathLenConstraint_specs_3 = {
	0,	0,	0,	0,	0,
	0,	/* Native long size */
	1	/* Unsigned representation */
};
static const ber_tlv_tag_t asn_DEF_pathLenConstraint_tags_3[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (2 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_pathLenConstraint_3 = {
	"pathLenConstraint",
	"pathLenConstraint",
	&asn_OP_NativeInteger,
	asn_DEF_pathLenConstraint_tags_3,
	sizeof(asn_DEF_pathLenConstraint_tags_3)
		/sizeof(asn_DEF_pathLenConstraint_tags_3[0]), /* 1 */
	asn_DEF_pathLenConstraint_tags_3,	/* Same as above */
	sizeof(asn_DEF_pathLenConstraint_tags_3)
		/sizeof(asn_DEF_pathLenConstraint_tags_3[0]), /* 1 */
	{ &asn_OER_type_pathLenConstraint_constr_3, &asn_PER_type_pathLenConstraint_constr_3, pathLenConstraint_3_constraint },
	0, 0,	/* No members */
	&asn_SPC_pathLenConstraint_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_BasicConstraints_1[] = {
	{ ATF_NOFLAGS, 2, offsetof(struct BasicConstraints, cA),
		(ASN_TAG_CLASS_UNIVERSAL | (1 << 2)),
		0,
		&asn_DEF_BOOLEAN,
		0,
		{ 0, 0, 0 },
		&asn_DFL_2_cmp_0,	/* Compare DEFAULT 0 */
		&asn_DFL_2_set_0,	/* Set DEFAULT 0 */
		"cA"
		},
	{ ATF_POINTER, 1, offsetof(struct BasicConstraints, pathLenConstraint),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_pathLenConstraint_3,
		0,
		{ &asn_OER_memb_pathLenConstraint_constr_3, &asn_PER_memb_pathLenConstraint_constr_3,  memb_pathLenConstraint_constraint_1 },
		0, 0, /* No default value */
		"pathLenConstraint"
		},
};
static const int asn_MAP_BasicConstraints_oms_1[] = { 0, 1 };
static const ber_tlv_tag_t asn_DEF_BasicConstraints_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_BasicConstraints_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (1 << 2)), 0, 0, 0 }, /* cA */
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 1, 0, 0 } /* pathLenConstraint */
};
static asn_SEQUENCE_specifics_t asn_SPC_BasicConstraints_specs_1 = {
	sizeof(struct BasicConstraints),
	offsetof(struct BasicConstraints, _asn_ctx),
	asn_MAP_BasicConstraints_tag2el_1,
	2,	/* Count of tags in the map */
	asn_MAP_BasicConstraints_oms_1,	/* Optional members */
	2, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_BasicConstraints = {
	"BasicConstraints",
	"BasicConstraints",
	&asn_OP_SEQUENCE,
	asn_DEF_BasicConstraints_tags_1,
	sizeof(asn_DEF_BasicConstraints_tags_1)
		/sizeof(asn_DEF_BasicConstraints_tags_1[0]), /* 1 */
	asn_DEF_BasicConstraints_tags_1,	/* Same as above */
	sizeof(asn_DEF_BasicConstraints_tags_1)
		/sizeof(asn_DEF_BasicConstraints_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_BasicConstraints_1,
	2,	/* Elements count */
	&asn_SPC_BasicConstraints_specs_1	/* Additional specs */
};

