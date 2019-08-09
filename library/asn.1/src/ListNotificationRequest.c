/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#include "ListNotificationRequest.h"

static asn_TYPE_member_t asn_MBR_ListNotificationRequest_1[] = {
	{ ATF_POINTER, 1, offsetof(struct ListNotificationRequest, profileManagementOperation),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NotificationEvent,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"profileManagementOperation"
		},
};
static const int asn_MAP_ListNotificationRequest_oms_1[] = { 0 };
static const ber_tlv_tag_t asn_DEF_ListNotificationRequest_tags_1[] = {
	(ASN_TAG_CLASS_CONTEXT | (40 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_ListNotificationRequest_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 } /* profileManagementOperation */
};
static asn_SEQUENCE_specifics_t asn_SPC_ListNotificationRequest_specs_1 = {
	sizeof(struct ListNotificationRequest),
	offsetof(struct ListNotificationRequest, _asn_ctx),
	asn_MAP_ListNotificationRequest_tag2el_1,
	1,	/* Count of tags in the map */
	asn_MAP_ListNotificationRequest_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_ListNotificationRequest = {
	"ListNotificationRequest",
	"ListNotificationRequest",
	&asn_OP_SEQUENCE,
	asn_DEF_ListNotificationRequest_tags_1,
	sizeof(asn_DEF_ListNotificationRequest_tags_1)
		/sizeof(asn_DEF_ListNotificationRequest_tags_1[0]) - 1, /* 1 */
	asn_DEF_ListNotificationRequest_tags_1,	/* Same as above */
	sizeof(asn_DEF_ListNotificationRequest_tags_1)
		/sizeof(asn_DEF_ListNotificationRequest_tags_1[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_ListNotificationRequest_1,
	1,	/* Elements count */
	&asn_SPC_ListNotificationRequest_specs_1	/* Additional specs */
};

