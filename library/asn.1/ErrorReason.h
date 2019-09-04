/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_ErrorReason_H_
#define	_ErrorReason_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ErrorReason {
	ErrorReason_incorrectInputValues	= 1,
	ErrorReason_invalidSignature	= 2,
	ErrorReason_invalidTransactionId	= 3,
	ErrorReason_unsupportedCrtValues	= 4,
	ErrorReason_unsupportedRemoteOperationType	= 5,
	ErrorReason_unsupportedProfileClass	= 6,
	ErrorReason_scp03tStructureError	= 7,
	ErrorReason_scp03tSecurityError	= 8,
	ErrorReason_installFailedDueToIccidAlreadyExistsOnEuicc	= 9,
	ErrorReason_installFailedDueToInsufficientMemoryForProfile	= 10,
	ErrorReason_installFailedDueToInterruption	= 11,
	ErrorReason_installFailedDueToPEProcessingError	= 12,
	ErrorReason_installFailedDueToDataMismatch	= 13,
	ErrorReason_testProfileInstallFailedDueToInvalidNaaKey	= 14,
	ErrorReason_pprNotAllowed	= 15,
	ErrorReason_installFailedDueToUnknownError	= 127
} e_ErrorReason;

/* ErrorReason */
typedef long	 ErrorReason_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ErrorReason;
asn_struct_free_f ErrorReason_free;
asn_struct_print_f ErrorReason_print;
asn_constr_check_f ErrorReason_constraint;
ber_type_decoder_f ErrorReason_decode_ber;
der_type_encoder_f ErrorReason_encode_der;
xer_type_decoder_f ErrorReason_decode_xer;
xer_type_encoder_f ErrorReason_encode_xer;
oer_type_decoder_f ErrorReason_decode_oer;
oer_type_encoder_f ErrorReason_encode_oer;
per_type_decoder_f ErrorReason_decode_uper;
per_type_encoder_f ErrorReason_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _ErrorReason_H_ */
#include <asn_internal.h>
