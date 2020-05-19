/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Personalize"
 * 	found in "Credential.asn"
 * 	`asn1c -fcompound-names`
 */

#include "SetRootKeyRequest.h"

static asn_TYPE_member_t asn_MBR_SetRootKeyRequest_1[] = {
	{ ATF_POINTER, 2, offsetof(struct SetRootKeyRequest, rootEccSk),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"rootEccSk"
		},
	{ ATF_POINTER, 1, offsetof(struct SetRootKeyRequest, rootAesKey),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"rootAesKey"
		},
};
static const int asn_MAP_SetRootKeyRequest_oms_1[] = { 0, 1 };
static const ber_tlv_tag_t asn_DEF_SetRootKeyRequest_tags_1[] = {
	(ASN_TAG_CLASS_PRIVATE | (32 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_SetRootKeyRequest_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* rootEccSk */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* rootAesKey */
};
static asn_SEQUENCE_specifics_t asn_SPC_SetRootKeyRequest_specs_1 = {
	sizeof(struct SetRootKeyRequest),
	offsetof(struct SetRootKeyRequest, _asn_ctx),
	asn_MAP_SetRootKeyRequest_tag2el_1,
	2,	/* Count of tags in the map */
	asn_MAP_SetRootKeyRequest_oms_1,	/* Optional members */
	2, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_SetRootKeyRequest = {
	"SetRootKeyRequest",
	"SetRootKeyRequest",
	&asn_OP_SEQUENCE,
	asn_DEF_SetRootKeyRequest_tags_1,
	sizeof(asn_DEF_SetRootKeyRequest_tags_1)
		/sizeof(asn_DEF_SetRootKeyRequest_tags_1[0]) - 1, /* 1 */
	asn_DEF_SetRootKeyRequest_tags_1,	/* Same as above */
	sizeof(asn_DEF_SetRootKeyRequest_tags_1)
		/sizeof(asn_DEF_SetRootKeyRequest_tags_1[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_SetRootKeyRequest_1,
	2,	/* Elements count */
	&asn_SPC_SetRootKeyRequest_specs_1	/* Additional specs */
};

