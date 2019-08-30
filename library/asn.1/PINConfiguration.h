/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PEDefinitions"
 * 	found in "PEDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_PINConfiguration_H_
#define	_PINConfiguration_H_


#include <asn_application.h>

/* Including external dependencies */
#include "PINKeyReferenceValue.h"
#include <OCTET_STRING.h>
#include "PUKKeyReferenceValue.h"
#include "UInt8.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PINConfiguration */
typedef struct PINConfiguration {
	PINKeyReferenceValue_t	 keyReference;
	OCTET_STRING_t	 pinValue;
	PUKKeyReferenceValue_t	*unblockingPINReference	/* OPTIONAL */;
	UInt8_t	*pinAttributes	/* DEFAULT 7 */;
	UInt8_t	*maxNumOfAttemps_retryNumLeft	/* DEFAULT 51 */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PINConfiguration_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PINConfiguration;
extern asn_SEQUENCE_specifics_t asn_SPC_PINConfiguration_specs_1;
extern asn_TYPE_member_t asn_MBR_PINConfiguration_1[5];

#ifdef __cplusplus
}
#endif

#endif	/* _PINConfiguration_H_ */
#include <asn_internal.h>
