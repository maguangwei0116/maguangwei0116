/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PEDefinitions"
 * 	found in "PEDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_ApplicationInstance_H_
#define	_ApplicationInstance_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ApplicationIdentifier.h"
#include <OCTET_STRING.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ApplicationSystemParameters;
struct UICCApplicationParameters;

/* ApplicationInstance */
typedef struct ApplicationInstance {
	ApplicationIdentifier_t	 applicationLoadPackageAID;
	ApplicationIdentifier_t	 classAID;
	ApplicationIdentifier_t	 instanceAID;
	ApplicationIdentifier_t	*extraditeSecurityDomainAID	/* OPTIONAL */;
	OCTET_STRING_t	 applicationPrivileges;
	OCTET_STRING_t	*lifeCycleState	/* DEFAULT '07'HH0000000'HH0000000000000000000000000000000000000000000000001000000000000000000000000000000020000000000000000000000000000000400000000000000000000000000000008'HH */;
	OCTET_STRING_t	 applicationSpecificParametersC9;
	struct ApplicationSystemParameters	*systemSpecificParameters	/* OPTIONAL */;
	struct UICCApplicationParameters	*applicationParameters	/* OPTIONAL */;
	struct ApplicationInstance__processData {
		A_SEQUENCE_OF(OCTET_STRING_t) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *processData;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ApplicationInstance_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ApplicationInstance;
extern asn_SEQUENCE_specifics_t asn_SPC_ApplicationInstance_specs_1;
extern asn_TYPE_member_t asn_MBR_ApplicationInstance_1[10];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ApplicationSystemParameters.h"
#include "UICCApplicationParameters.h"

#endif	/* _ApplicationInstance_H_ */
#include <asn_internal.h>
