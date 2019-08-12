/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_AuthenticateClientOkEs11_H_
#define	_AuthenticateClientOkEs11_H_


#include <asn_application.h>

/* Including external dependencies */
#include "TransactionId.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct EventEntries;

/* AuthenticateClientOkEs11 */
typedef struct AuthenticateClientOkEs11 {
	TransactionId_t	 transactionId;
	struct AuthenticateClientOkEs11__eventEntries {
		A_SEQUENCE_OF(struct EventEntries) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} eventEntries;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AuthenticateClientOkEs11_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AuthenticateClientOkEs11;
extern asn_SEQUENCE_specifics_t asn_SPC_AuthenticateClientOkEs11_specs_1;
extern asn_TYPE_member_t asn_MBR_AuthenticateClientOkEs11_1[2];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "EventEntries.h"

#endif	/* _AuthenticateClientOkEs11_H_ */
#include <asn_internal.h>
