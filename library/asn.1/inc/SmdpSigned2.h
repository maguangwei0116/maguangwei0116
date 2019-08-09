/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_SmdpSigned2_H_
#define	_SmdpSigned2_H_


#include <asn_application.h>

/* Including external dependencies */
#include "TransactionId.h"
#include <BOOLEAN.h>
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SmdpSigned2 */
typedef struct SmdpSigned2 {
	TransactionId_t	 transactionId;
	BOOLEAN_t	 ccRequiredFlag;
	OCTET_STRING_t	*bppEuiccOtpk	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SmdpSigned2_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SmdpSigned2;
extern asn_SEQUENCE_specifics_t asn_SPC_SmdpSigned2_specs_1;
extern asn_TYPE_member_t asn_MBR_SmdpSigned2_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _SmdpSigned2_H_ */
#include <asn_internal.h>
