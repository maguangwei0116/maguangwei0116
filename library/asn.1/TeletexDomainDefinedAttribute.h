/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PKIX1Explicit88"
 * 	found in "PKIX1Explicit88.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_TeletexDomainDefinedAttribute_H_
#define	_TeletexDomainDefinedAttribute_H_


#include <asn_application.h>

/* Including external dependencies */
#include <TeletexString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TeletexDomainDefinedAttribute */
typedef struct TeletexDomainDefinedAttribute {
	TeletexString_t	 type;
	TeletexString_t	 value;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TeletexDomainDefinedAttribute_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TeletexDomainDefinedAttribute;
extern asn_SEQUENCE_specifics_t asn_SPC_TeletexDomainDefinedAttribute_specs_1;
extern asn_TYPE_member_t asn_MBR_TeletexDomainDefinedAttribute_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _TeletexDomainDefinedAttribute_H_ */
#include <asn_internal.h>
