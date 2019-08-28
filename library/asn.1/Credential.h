/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Credential"
 * 	found in "Credential.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_Credential_H_
#define	_Credential_H_


#include <asn_application.h>

/* Including external dependencies */
#include "TBHCredential.h"
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Credential */
typedef struct Credential {
	TBHCredential_t	 tbhCredential;
	OCTET_STRING_t	 hashCode;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Credential_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Credential;

#ifdef __cplusplus
}
#endif

#endif	/* _Credential_H_ */
#include <asn_internal.h>
