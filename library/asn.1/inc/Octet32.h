/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_Octet32_H_
#define	_Octet32_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Octet32 */
typedef OCTET_STRING_t	 Octet32_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_Octet32_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_Octet32;
asn_struct_free_f Octet32_free;
asn_struct_print_f Octet32_print;
asn_constr_check_f Octet32_constraint;
ber_type_decoder_f Octet32_decode_ber;
der_type_encoder_f Octet32_encode_der;
xer_type_decoder_f Octet32_decode_xer;
xer_type_encoder_f Octet32_encode_xer;
oer_type_decoder_f Octet32_decode_oer;
oer_type_encoder_f Octet32_encode_oer;
per_type_decoder_f Octet32_decode_uper;
per_type_encoder_f Octet32_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _Octet32_H_ */
#include <asn_internal.h>
