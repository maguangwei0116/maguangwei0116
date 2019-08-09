/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_GetBoundProfilePackageRequest_H_
#define	_GetBoundProfilePackageRequest_H_


#include <asn_application.h>

/* Including external dependencies */
#include "TransactionId.h"
#include "PrepareDownloadResponse.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GetBoundProfilePackageRequest */
typedef struct GetBoundProfilePackageRequest {
	TransactionId_t	 transactionId;
	PrepareDownloadResponse_t	 prepareDownloadResponse;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GetBoundProfilePackageRequest_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GetBoundProfilePackageRequest;
extern asn_SEQUENCE_specifics_t asn_SPC_GetBoundProfilePackageRequest_specs_1;
extern asn_TYPE_member_t asn_MBR_GetBoundProfilePackageRequest_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _GetBoundProfilePackageRequest_H_ */
#include <asn_internal.h>
