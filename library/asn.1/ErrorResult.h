/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_ErrorResult_H_
#define	_ErrorResult_H_


#include <asn_application.h>

/* Including external dependencies */
#include "BppCommandId.h"
#include "ErrorReason.h"
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ErrorResult */
typedef struct ErrorResult {
	BppCommandId_t	 bppCommandId;
	ErrorReason_t	 errorReason;
	OCTET_STRING_t	*simaResponse	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ErrorResult_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ErrorResult;
extern asn_SEQUENCE_specifics_t asn_SPC_ErrorResult_specs_1;
extern asn_TYPE_member_t asn_MBR_ErrorResult_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _ErrorResult_H_ */
#include <asn_internal.h>
