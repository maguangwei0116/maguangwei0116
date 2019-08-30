/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PKIX1Implicit88"
 * 	found in "PKIX1Implicit88.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_PolicyQualifierInfo_H_
#define	_PolicyQualifierInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include "PolicyQualifierId.h"
#include <ANY.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PolicyQualifierInfo */
typedef struct PolicyQualifierInfo {
	PolicyQualifierId_t	 policyQualifierId;
	ANY_t	 qualifier;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PolicyQualifierInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PolicyQualifierInfo;
extern asn_SEQUENCE_specifics_t asn_SPC_PolicyQualifierInfo_specs_1;
extern asn_TYPE_member_t asn_MBR_PolicyQualifierInfo_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _PolicyQualifierInfo_H_ */
#include <asn_internal.h>
