/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_EuiccSigned1_H_
#define	_EuiccSigned1_H_


#include <asn_application.h>

/* Including external dependencies */
#include "TransactionId.h"
#include <UTF8String.h>
#include "Octet16.h"
#include "EUICCInfo2.h"
#include "CtxParams1.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* EuiccSigned1 */
typedef struct EuiccSigned1 {
	TransactionId_t	 transactionId;
	UTF8String_t	 serverAddress;
	Octet16_t	 serverChallenge;
	EUICCInfo2_t	 euiccInfo2;
	CtxParams1_t	 ctxParams1;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} EuiccSigned1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EuiccSigned1;
extern asn_SEQUENCE_specifics_t asn_SPC_EuiccSigned1_specs_1;
extern asn_TYPE_member_t asn_MBR_EuiccSigned1_1[5];

#ifdef __cplusplus
}
#endif

#endif	/* _EuiccSigned1_H_ */
#include <asn_internal.h>
