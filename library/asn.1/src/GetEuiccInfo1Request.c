/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "GetEuiccInfo1Request.h"

static const ber_tlv_tag_t asn_DEF_GetEuiccInfo1Request_tags_1[] = {
	(ASN_TAG_CLASS_CONTEXT | (32 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SEQUENCE_specifics_t asn_SPC_GetEuiccInfo1Request_specs_1 = {
	sizeof(struct GetEuiccInfo1Request),
	offsetof(struct GetEuiccInfo1Request, _asn_ctx),
	0,	/* No top level tags */
	0,	/* No tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_GetEuiccInfo1Request = {
	"GetEuiccInfo1Request",
	"GetEuiccInfo1Request",
	&asn_OP_SEQUENCE,
	asn_DEF_GetEuiccInfo1Request_tags_1,
	sizeof(asn_DEF_GetEuiccInfo1Request_tags_1)
		/sizeof(asn_DEF_GetEuiccInfo1Request_tags_1[0]) - 1, /* 1 */
	asn_DEF_GetEuiccInfo1Request_tags_1,	/* Same as above */
	sizeof(asn_DEF_GetEuiccInfo1Request_tags_1)
		/sizeof(asn_DEF_GetEuiccInfo1Request_tags_1[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	0, 0,	/* No members */
	&asn_SPC_GetEuiccInfo1Request_specs_1	/* Additional specs */
};

