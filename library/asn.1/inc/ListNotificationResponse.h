/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_ListNotificationResponse_H_
#define	_ListNotificationResponse_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ListNotificationResponse_PR {
	ListNotificationResponse_PR_NOTHING,	/* No components present */
	ListNotificationResponse_PR_notificationMetadataList,
	ListNotificationResponse_PR_listNotificationsResultError
} ListNotificationResponse_PR;
typedef enum ListNotificationResponse__listNotificationsResultError {
	ListNotificationResponse__listNotificationsResultError_undefinedError	= 127
} e_ListNotificationResponse__listNotificationsResultError;

/* Forward declarations */
struct NotificationMetadata;

/* ListNotificationResponse */
typedef struct ListNotificationResponse {
	ListNotificationResponse_PR present;
	union ListNotificationResponse_u {
		struct ListNotificationResponse__notificationMetadataList {
			A_SEQUENCE_OF(struct NotificationMetadata) list;
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} notificationMetadataList;
		long	 listNotificationsResultError;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ListNotificationResponse_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ListNotificationResponse;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "NotificationMetadata.h"

#endif	/* _ListNotificationResponse_H_ */
#include <asn_internal.h>
