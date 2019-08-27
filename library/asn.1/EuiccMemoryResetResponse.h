/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_EuiccMemoryResetResponse_H_
#define	_EuiccMemoryResetResponse_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum EuiccMemoryResetResponse__resetResult {
	EuiccMemoryResetResponse__resetResult_ok	= 0,
	EuiccMemoryResetResponse__resetResult_nothingToDelete	= 1,
	EuiccMemoryResetResponse__resetResult_catBusy	= 5,
	EuiccMemoryResetResponse__resetResult_undefinedError	= 127
} e_EuiccMemoryResetResponse__resetResult;

/* EuiccMemoryResetResponse */
typedef struct EuiccMemoryResetResponse {
	long	 resetResult;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} EuiccMemoryResetResponse_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EuiccMemoryResetResponse;

#ifdef __cplusplus
}
#endif

#endif	/* _EuiccMemoryResetResponse_H_ */
#include <asn_internal.h>
