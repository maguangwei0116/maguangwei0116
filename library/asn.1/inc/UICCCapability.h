/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "RSPDefinitionsV2.1.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_UICCCapability_H_
#define	_UICCCapability_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum UICCCapability {
	UICCCapability_contactlessSupport	= 0,
	UICCCapability_usimSupport	= 1,
	UICCCapability_isimSupport	= 2,
	UICCCapability_csimSupport	= 3,
	UICCCapability_akaMilenage	= 4,
	UICCCapability_akaCave	= 5,
	UICCCapability_akaTuak128	= 6,
	UICCCapability_akaTuak256	= 7,
	UICCCapability_rfu1	= 8,
	UICCCapability_rfu2	= 9,
	UICCCapability_gbaAuthenUsim	= 10,
	UICCCapability_gbaAuthenISim	= 11,
	UICCCapability_mbmsAuthenUsim	= 12,
	UICCCapability_eapClient	= 13,
	UICCCapability_javacard	= 14,
	UICCCapability_multos	= 15,
	UICCCapability_multipleUsimSupport	= 16,
	UICCCapability_multipleIsimSupport	= 17,
	UICCCapability_multipleCsimSupport	= 18
} e_UICCCapability;

/* UICCCapability */
typedef BIT_STRING_t	 UICCCapability_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UICCCapability;
asn_struct_free_f UICCCapability_free;
asn_struct_print_f UICCCapability_print;
asn_constr_check_f UICCCapability_constraint;
ber_type_decoder_f UICCCapability_decode_ber;
der_type_encoder_f UICCCapability_encode_der;
xer_type_decoder_f UICCCapability_decode_xer;
xer_type_encoder_f UICCCapability_encode_xer;
oer_type_decoder_f UICCCapability_decode_oer;
oer_type_encoder_f UICCCapability_encode_oer;
per_type_decoder_f UICCCapability_decode_uper;
per_type_encoder_f UICCCapability_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _UICCCapability_H_ */
#include <asn_internal.h>
