/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PEDefinitions"
 * 	found in "PEDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "ADFRFMAccess.h"

asn_TYPE_member_t asn_MBR_ADFRFMAccess_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ADFRFMAccess, adfAID),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ApplicationIdentifier,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"adfAID"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ADFRFMAccess, adfAccessDomain),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"adfAccessDomain"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ADFRFMAccess, adfAdminAccessDomain),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"adfAdminAccessDomain"
		},
};
static const ber_tlv_tag_t asn_DEF_ADFRFMAccess_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ADFRFMAccess_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* adfAID */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* adfAccessDomain */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* adfAdminAccessDomain */
};
asn_SEQUENCE_specifics_t asn_SPC_ADFRFMAccess_specs_1 = {
	sizeof(struct ADFRFMAccess),
	offsetof(struct ADFRFMAccess, _asn_ctx),
	asn_MAP_ADFRFMAccess_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	3,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_ADFRFMAccess = {
	"ADFRFMAccess",
	"ADFRFMAccess",
	&asn_OP_SEQUENCE,
	asn_DEF_ADFRFMAccess_tags_1,
	sizeof(asn_DEF_ADFRFMAccess_tags_1)
		/sizeof(asn_DEF_ADFRFMAccess_tags_1[0]), /* 1 */
	asn_DEF_ADFRFMAccess_tags_1,	/* Same as above */
	sizeof(asn_DEF_ADFRFMAccess_tags_1)
		/sizeof(asn_DEF_ADFRFMAccess_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_ADFRFMAccess_1,
	3,	/* Elements count */
	&asn_SPC_ADFRFMAccess_specs_1	/* Additional specs */
};

