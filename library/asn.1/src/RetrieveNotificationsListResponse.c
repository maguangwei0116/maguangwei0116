/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#include "RetrieveNotificationsListResponse.h"

static asn_oer_constraints_t asn_OER_type_RetrieveNotificationsListResponse_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_RetrieveNotificationsListResponse_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  1,  1,  0,  1 }	/* (0..1,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_notificationList_2[] = {
	{ ATF_POINTER, 0, 0,
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_PendingNotification,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		""
		},
};
static const ber_tlv_tag_t asn_DEF_notificationList_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_notificationList_specs_2 = {
	sizeof(struct RetrieveNotificationsListResponse__notificationList),
	offsetof(struct RetrieveNotificationsListResponse__notificationList, _asn_ctx),
	2,	/* XER encoding is XMLValueList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_notificationList_2 = {
	"notificationList",
	"notificationList",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_notificationList_tags_2,
	sizeof(asn_DEF_notificationList_tags_2)
		/sizeof(asn_DEF_notificationList_tags_2[0]) - 1, /* 1 */
	asn_DEF_notificationList_tags_2,	/* Same as above */
	sizeof(asn_DEF_notificationList_tags_2)
		/sizeof(asn_DEF_notificationList_tags_2[0]), /* 2 */
	{ 0, 0, SEQUENCE_OF_constraint },
	asn_MBR_notificationList_2,
	1,	/* Single element */
	&asn_SPC_notificationList_specs_2	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_RetrieveNotificationsListResponse_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct RetrieveNotificationsListResponse, choice.notificationList),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		0,
		&asn_DEF_notificationList_2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"notificationList"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct RetrieveNotificationsListResponse, choice.notificationsListResultError),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"notificationsListResultError"
		},
};
static const ber_tlv_tag_t asn_DEF_RetrieveNotificationsListResponse_tags_1[] = {
	(ASN_TAG_CLASS_CONTEXT | (43 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_RetrieveNotificationsListResponse_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* notificationList */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* notificationsListResultError */
};
static asn_CHOICE_specifics_t asn_SPC_RetrieveNotificationsListResponse_specs_1 = {
	sizeof(struct RetrieveNotificationsListResponse),
	offsetof(struct RetrieveNotificationsListResponse, _asn_ctx),
	offsetof(struct RetrieveNotificationsListResponse, present),
	sizeof(((struct RetrieveNotificationsListResponse *)0)->present),
	asn_MAP_RetrieveNotificationsListResponse_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0,
	2	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_RetrieveNotificationsListResponse = {
	"RetrieveNotificationsListResponse",
	"RetrieveNotificationsListResponse",
	&asn_OP_CHOICE,
	asn_DEF_RetrieveNotificationsListResponse_tags_1,
	sizeof(asn_DEF_RetrieveNotificationsListResponse_tags_1)
		/sizeof(asn_DEF_RetrieveNotificationsListResponse_tags_1[0]), /* 1 */
	asn_DEF_RetrieveNotificationsListResponse_tags_1,	/* Same as above */
	sizeof(asn_DEF_RetrieveNotificationsListResponse_tags_1)
		/sizeof(asn_DEF_RetrieveNotificationsListResponse_tags_1[0]), /* 1 */
	{ &asn_OER_type_RetrieveNotificationsListResponse_constr_1, &asn_PER_type_RetrieveNotificationsListResponse_constr_1, CHOICE_constraint },
	asn_MBR_RetrieveNotificationsListResponse_1,
	2,	/* Elements count */
	&asn_SPC_RetrieveNotificationsListResponse_specs_1	/* Additional specs */
};

