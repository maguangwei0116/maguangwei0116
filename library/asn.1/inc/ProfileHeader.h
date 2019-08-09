/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "PEDefinitions"
 * 	found in "PEDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_ProfileHeader_H_
#define	_ProfileHeader_H_


#include <asn_application.h>

/* Including external dependencies */
#include "UInt8.h"
#include <UTF8String.h>
#include <OCTET_STRING.h>
#include "ServicesList.h"
#include <OBJECT_IDENTIFIER.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include "ApplicationIdentifier.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward definitions */
typedef struct ProfileHeader__eUICC_Mandatory_AIDs__Member {
	ApplicationIdentifier_t	 aid;
	OCTET_STRING_t	 version;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ProfileHeader__eUICC_Mandatory_AIDs__Member;

/* ProfileHeader */
typedef struct ProfileHeader {
	UInt8_t	 major_version;
	UInt8_t	 minor_version;
	UTF8String_t	*profileType	/* OPTIONAL */;
	OCTET_STRING_t	 iccid;
	OCTET_STRING_t	*pol	/* OPTIONAL */;
	ServicesList_t	 eUICC_Mandatory_services;
	struct ProfileHeader__eUICC_Mandatory_GFSTEList {
		A_SEQUENCE_OF(OBJECT_IDENTIFIER_t) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} eUICC_Mandatory_GFSTEList;
	OCTET_STRING_t	*connectivityParameters	/* OPTIONAL */;
	struct ProfileHeader__eUICC_Mandatory_AIDs {
		A_SEQUENCE_OF(ProfileHeader__eUICC_Mandatory_AIDs__Member) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *eUICC_Mandatory_AIDs;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ProfileHeader_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ProfileHeader;
extern asn_SEQUENCE_specifics_t asn_SPC_ProfileHeader_specs_1;
extern asn_TYPE_member_t asn_MBR_ProfileHeader_1[9];

#ifdef __cplusplus
}
#endif

#endif	/* _ProfileHeader_H_ */
#include <asn_internal.h>
