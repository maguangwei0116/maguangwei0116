/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.2.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_Octet8_H_
#define	_Octet8_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Octet8 */
typedef OCTET_STRING_t	 Octet8_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_Octet8_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_Octet8;
asn_struct_free_f Octet8_free;
asn_struct_print_f Octet8_print;
asn_constr_check_f Octet8_constraint;
ber_type_decoder_f Octet8_decode_ber;
der_type_encoder_f Octet8_encode_der;
xer_type_decoder_f Octet8_decode_xer;
xer_type_encoder_f Octet8_encode_xer;
oer_type_decoder_f Octet8_decode_oer;
oer_type_encoder_f Octet8_encode_oer;
per_type_decoder_f Octet8_decode_uper;
per_type_encoder_f Octet8_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _Octet8_H_ */
#include <asn_internal.h>
