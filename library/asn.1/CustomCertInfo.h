/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Personalize"
 * 	found in "Credential.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_CustomCertInfo_H_
#define	_CustomCertInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CustomCertInfo */
typedef struct CustomCertInfo {
	OCTET_STRING_t	 ciPkId;
	OCTET_STRING_t	 ciPk;
	OCTET_STRING_t	 euiccCrt;
	OCTET_STRING_t	 eumCrt;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CustomCertInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CustomCertInfo;
extern asn_SEQUENCE_specifics_t asn_SPC_CustomCertInfo_specs_1;
extern asn_TYPE_member_t asn_MBR_CustomCertInfo_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _CustomCertInfo_H_ */
#include <asn_internal.h>
