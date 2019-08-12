/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_AuthenticateServerResponse_H_
#define	_AuthenticateServerResponse_H_


#include <asn_application.h>

/* Including external dependencies */
#include "AuthenticateResponseOk.h"
#include "AuthenticateResponseError.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AuthenticateServerResponse_PR {
	AuthenticateServerResponse_PR_NOTHING,	/* No components present */
	AuthenticateServerResponse_PR_authenticateResponseOk,
	AuthenticateServerResponse_PR_authenticateResponseError
} AuthenticateServerResponse_PR;

/* AuthenticateServerResponse */
typedef struct AuthenticateServerResponse {
	AuthenticateServerResponse_PR present;
	union AuthenticateServerResponse_u {
		AuthenticateResponseOk_t	 authenticateResponseOk;
		AuthenticateResponseError_t	 authenticateResponseError;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AuthenticateServerResponse_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AuthenticateServerResponse;
extern asn_CHOICE_specifics_t asn_SPC_AuthenticateServerResponse_specs_1;
extern asn_TYPE_member_t asn_MBR_AuthenticateServerResponse_1[2];
extern asn_per_constraints_t asn_PER_type_AuthenticateServerResponse_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _AuthenticateServerResponse_H_ */
#include <asn_internal.h>
