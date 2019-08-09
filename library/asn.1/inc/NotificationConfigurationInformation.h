/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_NotificationConfigurationInformation_H_
#define	_NotificationConfigurationInformation_H_


#include <asn_application.h>

/* Including external dependencies */
#include "NotificationEvent.h"
#include <UTF8String.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NotificationConfigurationInformation */
typedef struct NotificationConfigurationInformation {
	NotificationEvent_t	 profileManagementOperation;
	UTF8String_t	 notificationAddress;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NotificationConfigurationInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NotificationConfigurationInformation;
extern asn_SEQUENCE_specifics_t asn_SPC_NotificationConfigurationInformation_specs_1;
extern asn_TYPE_member_t asn_MBR_NotificationConfigurationInformation_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _NotificationConfigurationInformation_H_ */
#include <asn_internal.h>
