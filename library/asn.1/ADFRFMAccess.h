/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PEDefinitions"
 * 	found in "PEDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_ADFRFMAccess_H_
#define	_ADFRFMAccess_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ApplicationIdentifier.h"
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ADFRFMAccess */
typedef struct ADFRFMAccess {
	ApplicationIdentifier_t	 adfAID;
	OCTET_STRING_t	 adfAccessDomain;
	OCTET_STRING_t	 adfAdminAccessDomain;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ADFRFMAccess_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ADFRFMAccess;
extern asn_SEQUENCE_specifics_t asn_SPC_ADFRFMAccess_specs_1;
extern asn_TYPE_member_t asn_MBR_ADFRFMAccess_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _ADFRFMAccess_H_ */
#include <asn_internal.h>
