/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_CtxParams1_H_
#define	_CtxParams1_H_


#include <asn_application.h>

/* Including external dependencies */
#include "CtxParamsForCommonAuthentication.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CtxParams1_PR {
	CtxParams1_PR_NOTHING,	/* No components present */
	CtxParams1_PR_ctxParamsForCommonAuthentication
} CtxParams1_PR;

/* CtxParams1 */
typedef struct CtxParams1 {
	CtxParams1_PR present;
	union CtxParams1_u {
		CtxParamsForCommonAuthentication_t	 ctxParamsForCommonAuthentication;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CtxParams1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CtxParams1;
extern asn_CHOICE_specifics_t asn_SPC_CtxParams1_specs_1;
extern asn_TYPE_member_t asn_MBR_CtxParams1_1[1];
extern asn_per_constraints_t asn_PER_type_CtxParams1_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _CtxParams1_H_ */
#include <asn_internal.h>
