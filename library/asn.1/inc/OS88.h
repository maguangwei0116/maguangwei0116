/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_OS88_H_
#define	_OS88_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* OS88 */
typedef OCTET_STRING_t	 OS88_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_OS88;
asn_struct_free_f OS88_free;
asn_struct_print_f OS88_print;
asn_constr_check_f OS88_constraint;
ber_type_decoder_f OS88_decode_ber;
der_type_encoder_f OS88_encode_der;
xer_type_decoder_f OS88_decode_xer;
xer_type_encoder_f OS88_encode_xer;
oer_type_decoder_f OS88_decode_oer;
oer_type_encoder_f OS88_encode_oer;
per_type_decoder_f OS88_decode_uper;
per_type_encoder_f OS88_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _OS88_H_ */
#include <asn_internal.h>
