/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PEDefinitions"
 * 	found in "PEDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "UICCApplicationParameters.h"

asn_TYPE_member_t asn_MBR_UICCApplicationParameters_1[] = {
	{ ATF_POINTER, 3, offsetof(struct UICCApplicationParameters, uiccToolkitApplicationSpecificParametersField),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"uiccToolkitApplicationSpecificParametersField"
		},
	{ ATF_POINTER, 2, offsetof(struct UICCApplicationParameters, uiccAccessApplicationSpecificParametersField),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"uiccAccessApplicationSpecificParametersField"
		},
	{ ATF_POINTER, 1, offsetof(struct UICCApplicationParameters, uiccAdministrativeAccessApplicationSpecificParametersField),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"uiccAdministrativeAccessApplicationSpecificParametersField"
		},
};
static const int asn_MAP_UICCApplicationParameters_oms_1[] = { 0, 1, 2 };
static const ber_tlv_tag_t asn_DEF_UICCApplicationParameters_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_UICCApplicationParameters_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* uiccToolkitApplicationSpecificParametersField */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* uiccAccessApplicationSpecificParametersField */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* uiccAdministrativeAccessApplicationSpecificParametersField */
};
asn_SEQUENCE_specifics_t asn_SPC_UICCApplicationParameters_specs_1 = {
	sizeof(struct UICCApplicationParameters),
	offsetof(struct UICCApplicationParameters, _asn_ctx),
	asn_MAP_UICCApplicationParameters_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_UICCApplicationParameters_oms_1,	/* Optional members */
	3, 0,	/* Root/Additions */
	3,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_UICCApplicationParameters = {
	"UICCApplicationParameters",
	"UICCApplicationParameters",
	&asn_OP_SEQUENCE,
	asn_DEF_UICCApplicationParameters_tags_1,
	sizeof(asn_DEF_UICCApplicationParameters_tags_1)
		/sizeof(asn_DEF_UICCApplicationParameters_tags_1[0]), /* 1 */
	asn_DEF_UICCApplicationParameters_tags_1,	/* Same as above */
	sizeof(asn_DEF_UICCApplicationParameters_tags_1)
		/sizeof(asn_DEF_UICCApplicationParameters_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_UICCApplicationParameters_1,
	3,	/* Elements count */
	&asn_SPC_UICCApplicationParameters_specs_1	/* Additional specs */
};

