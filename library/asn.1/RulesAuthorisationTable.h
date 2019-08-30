/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_RulesAuthorisationTable_H_
#define	_RulesAuthorisationTable_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ProfilePolicyAuthorisationRule;

/* RulesAuthorisationTable */
typedef struct RulesAuthorisationTable {
	A_SEQUENCE_OF(struct ProfilePolicyAuthorisationRule) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RulesAuthorisationTable_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RulesAuthorisationTable;
extern asn_SET_OF_specifics_t asn_SPC_RulesAuthorisationTable_specs_1;
extern asn_TYPE_member_t asn_MBR_RulesAuthorisationTable_1[1];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ProfilePolicyAuthorisationRule.h"

#endif	/* _RulesAuthorisationTable_H_ */
#include <asn_internal.h>
