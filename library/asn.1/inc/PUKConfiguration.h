/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PEDefinitions"
 * 	found in "PEDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_PUKConfiguration_H_
#define	_PUKConfiguration_H_


#include <asn_application.h>

/* Including external dependencies */
#include "PUKKeyReferenceValue.h"
#include <OCTET_STRING.h>
#include "UInt8.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PUKConfiguration */
typedef struct PUKConfiguration {
	PUKKeyReferenceValue_t	 keyReference;
	OCTET_STRING_t	 pukValue;
	UInt8_t	*maxNumOfAttemps_retryNumLeft	/* DEFAULT 170 */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PUKConfiguration_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PUKConfiguration;
extern asn_SEQUENCE_specifics_t asn_SPC_PUKConfiguration_specs_1;
extern asn_TYPE_member_t asn_MBR_PUKConfiguration_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _PUKConfiguration_H_ */
#include <asn_internal.h>
