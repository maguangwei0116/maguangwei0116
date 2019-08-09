/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_CancelSessionResponse_H_
#define	_CancelSessionResponse_H_


#include <asn_application.h>

/* Including external dependencies */
#include "CancelSessionResponseOk.h"
#include <NativeInteger.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CancelSessionResponse_PR {
	CancelSessionResponse_PR_NOTHING,	/* No components present */
	CancelSessionResponse_PR_cancelSessionResponseOk,
	CancelSessionResponse_PR_cancelSessionResponseError
} CancelSessionResponse_PR;
typedef enum CancelSessionResponse__cancelSessionResponseError {
	CancelSessionResponse__cancelSessionResponseError_invalidTransactionId	= 5,
	CancelSessionResponse__cancelSessionResponseError_undefinedError	= 127
} e_CancelSessionResponse__cancelSessionResponseError;

/* CancelSessionResponse */
typedef struct CancelSessionResponse {
	CancelSessionResponse_PR present;
	union CancelSessionResponse_u {
		CancelSessionResponseOk_t	 cancelSessionResponseOk;
		long	 cancelSessionResponseError;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CancelSessionResponse_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CancelSessionResponse;
extern asn_CHOICE_specifics_t asn_SPC_CancelSessionResponse_specs_1;
extern asn_TYPE_member_t asn_MBR_CancelSessionResponse_1[2];
extern asn_per_constraints_t asn_PER_type_CancelSessionResponse_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _CancelSessionResponse_H_ */
#include <asn_internal.h>
