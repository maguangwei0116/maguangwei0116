/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PROFILE"
 * 	found in "ShareProfile.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_TBHRequest_H_
#define	_TBHRequest_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TBHRequest */
typedef struct TBHRequest {
	OCTET_STRING_t	 iccid;
	OCTET_STRING_t	 imsi;
	OCTET_STRING_t	 key;
	OCTET_STRING_t	 opc;
	OCTET_STRING_t	*rotation	/* OPTIONAL */;
	OCTET_STRING_t	*xoring	/* OPTIONAL */;
	OCTET_STRING_t	*sqnFlag	/* OPTIONAL */;
	OCTET_STRING_t	*rplmn	/* OPTIONAL */;
	OCTET_STRING_t	*fplmn	/* OPTIONAL */;
	OCTET_STRING_t	*hplmn	/* OPTIONAL */;
	OCTET_STRING_t	*ehplmn	/* OPTIONAL */;
	OCTET_STRING_t	*oplmn	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TBHRequest_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TBHRequest;
extern asn_SEQUENCE_specifics_t asn_SPC_TBHRequest_specs_1;
extern asn_TYPE_member_t asn_MBR_TBHRequest_1[12];

#ifdef __cplusplus
}
#endif

#endif	/* _TBHRequest_H_ */
#include <asn_internal.h>
