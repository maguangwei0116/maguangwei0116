/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "BootstrapRequest"
 * 	found in "ShareProfile.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_BootstrapRequest_BootstrapRequest_H_
#define	_BootstrapRequest_BootstrapRequest_H_


#include <asn_application.h>

/* Including external dependencies */
#include "BootstrapRequest_TBHRequest.h"
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BootstrapRequest */
typedef struct BootstrapRequest_BootstrapRequest {
	BootstrapRequest_TBHRequest_t	 tbhRequest;
	OCTET_STRING_t	 hashCode;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} BootstrapRequest_BootstrapRequest_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BootstrapRequest_BootstrapRequest;

#ifdef __cplusplus
}
#endif

#endif	/* _BootstrapRequest_BootstrapRequest_H_ */
#include <asn_internal.h>
