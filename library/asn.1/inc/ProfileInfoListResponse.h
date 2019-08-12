/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_ProfileInfoListResponse_H_
#define	_ProfileInfoListResponse_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ProfileInfoListError.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ProfileInfoListResponse_PR {
	ProfileInfoListResponse_PR_NOTHING,	/* No components present */
	ProfileInfoListResponse_PR_profileInfoListOk,
	ProfileInfoListResponse_PR_profileInfoListError
} ProfileInfoListResponse_PR;

/* Forward declarations */
struct ProfileInfo;

/* ProfileInfoListResponse */
typedef struct ProfileInfoListResponse {
	ProfileInfoListResponse_PR present;
	union ProfileInfoListResponse_u {
		struct ProfileInfoListResponse__profileInfoListOk {
			A_SEQUENCE_OF(struct ProfileInfo) list;
			
			/* Context for parsing across buffer boundaries */
			asn_struct_ctx_t _asn_ctx;
		} profileInfoListOk;
		ProfileInfoListError_t	 profileInfoListError;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ProfileInfoListResponse_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ProfileInfoListResponse;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ProfileInfo.h"

#endif	/* _ProfileInfoListResponse_H_ */
#include <asn_internal.h>
