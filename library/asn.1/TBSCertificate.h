/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PKIX1Explicit88"
 * 	found in "PKIX1Explicit88.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_TBSCertificate_H_
#define	_TBSCertificate_H_


#include <asn_application.h>

/* Including external dependencies */
#include "Version.h"
#include "CertificateSerialNumber.h"
#include "AlgorithmIdentifier.h"
#include "Name.h"
#include "Validity.h"
#include "SubjectPublicKeyInfo.h"
#include "UniqueIdentifier.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Extensions;

/* TBSCertificate */
typedef struct TBSCertificate {
	Version_t	 version	/* DEFAULT 0 */;
	CertificateSerialNumber_t	 serialNumber;
	AlgorithmIdentifier_t	 signature;
	Name_t	 issuer;
	Validity_t	 validity;
	Name_t	 subject;
	SubjectPublicKeyInfo_t	 subjectPublicKeyInfo;
	UniqueIdentifier_t	*issuerUniqueID	/* OPTIONAL */;
	UniqueIdentifier_t	*subjectUniqueID	/* OPTIONAL */;
	struct Extensions	*extensions	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TBSCertificate_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TBSCertificate;
extern asn_SEQUENCE_specifics_t asn_SPC_TBSCertificate_specs_1;
extern asn_TYPE_member_t asn_MBR_TBSCertificate_1[10];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "Extensions.h"

#endif	/* _TBSCertificate_H_ */
#include <asn_internal.h>
