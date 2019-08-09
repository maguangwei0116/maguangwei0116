/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_CancelSessionResponseOk_H_
#define	_CancelSessionResponseOk_H_


#include <asn_application.h>

/* Including external dependencies */
#include "EuiccCancelSessionSigned.h"
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CancelSessionResponseOk */
typedef struct CancelSessionResponseOk {
	EuiccCancelSessionSigned_t	 euiccCancelSessionSigned;
	OCTET_STRING_t	 euiccCancelSessionSignature;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CancelSessionResponseOk_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CancelSessionResponseOk;
extern asn_SEQUENCE_specifics_t asn_SPC_CancelSessionResponseOk_specs_1;
extern asn_TYPE_member_t asn_MBR_CancelSessionResponseOk_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _CancelSessionResponseOk_H_ */
#include <asn_internal.h>
