/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : bertlv.h
 * Date        : 2021.01.21
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __BERTLV_H__
#define __BERTLV_H__
 
#ifdef _cplusplus
extern "C" {
#endif

#include <rt_type.h>

/**
 * ASN.1 Tag definition
 */
#define ASN1_TAG_UNDEF                      -1
#define ASN1_TAG_EOC                        0
#define ASN1_TAG_BOOLEAN                    1
#define ASN1_TAG_INTEGER                    2
#define ASN1_TAG_BIT_STRING                 3
#define ASN1_TAG_OCTET_STRING               4
#define ASN1_TAG_NULL                       5
#define ASN1_TAG_OBJECT                     6
#define ASN1_TAG_OBJECT_DESCRIPTOR          7
#define ASN1_TAG_EXTERNAL                   8
#define ASN1_TAG_REAL                       9
#define ASN1_TAG_ENUMERATED                 10
#define ASN1_TAG_UTF8STRING                 12
#define ASN1_TAG_SEQUENCE                   16
#define ASN1_TAG_SET                        17
#define ASN1_TAG_NUMERICSTRING              18
#define ASN1_TAG_PRINTABLESTRING            19
#define ASN1_TAG_T61STRING                  20
#define ASN1_TAG_TELETEXSTRING              20
#define ASN1_TAG_VIDEOTEXSTRING             21
#define ASN1_TAG_IA5STRING                  22
#define ASN1_TAG_UTCTIME                    23
#define ASN1_TAG_GENERALIZEDTIME            24
#define ASN1_TAG_GRAPHICSTRING              25
#define ASN1_TAG_ISO64STRING                26
#define ASN1_TAG_VISIBLESTRING              26
#define ASN1_TAG_GENERALSTRING              27
#define ASN1_TAG_UNIVERSALSTRING            28
#define ASN1_TAG_BMPSTRING                  30

/**
 * BER Tag definition
 */
#define BER_TAG_CLASS_UNIVERSAL             0x00
#define BER_TAG_CLASS_APPLICATION           0x40
#define BER_TAG_CLASS_CONTEXT_SPECIFIC      0x80
#define BER_TAG_CLASS_PRIVATE               0xC0
#define BER_TAG_CONSTRUCTED                 0x20

/**
 * BER-TLV definition
 */
#define BERTLV_INVALID_OFFSET               0xFFFFFFFF

/**
 * @brief                 Get size of tag
 * @param[in] tag         The pointer to the tag
 * @return uint8_t        The size of tag
 * @note                  
 */
uint8_t bertlv_get_tag_size(const uint8_t* tag);

/**
 * @brief                 Get tag value
 * @param[in] tag         The pointer to the tag
 * @param[out] tag_size   The pointer to the size of tag
 * @return uint8_t        The size of tag
 * @note                  Only support two-bytes tag
 */
uint16_t bertlv_get_tag(const uint8_t* tag, uint8_t* tag_size);

/**
 * @brief                 Get size of length and size of value
 * @param[in] len         The pointer to the length
 * @param[out] len_size   The size of length itself
 * @return uint32_t       The length value(size of value)
 * @note                  
 */
uint32_t bertlv_get_length(const uint8_t* len, uint8_t* len_size);

/**
 * @brief                 Calculate the size of special length value
 * @param[in] len         The length value
 * @return uint8_t        The size of length
 * @note                  
 */
uint8_t bertlv_calc_length_size(uint32_t len);


/**
 * @brief                 Set a length value to buffer according BER-TLV encode
 * @param[in] len         The length value
 * @param[out] buf        The buffer to set length
 * @return uint8_t        The size of length
 * @note
 */
uint8_t bertlv_set_length(uint32_t len, uint8_t* buf);

/**
 * @brief                 Find the special tag in a BER-TLV list
 * @param[in] tlv         The pointer of BER-TLV list
 * @param[in] tlv_len     Length of BER-TLV list
 * @param[in] tag         The tag of the TLV element to search
 * @param[in] occurrence  The occurrence number of the TLV element (1 for the first, 2 for the second...)
 * @return uint32_t       The offset of the tag was found
 * @note                  
 */
uint32_t bertlv_find_tag(const uint8_t* tlv, uint32_t tlv_len, uint16_t tag, uint8_t occurrence);

/**
 * @brief                 Calculate the size of T(ag) and L(ength)
 * @param[in] tlv         The pointer of TLV buffer
 * @param[out] value_len  The size of V(alue)
 * @return uint16_t       The size of T(ag) and L(ength)
 * @note                  
 */
uint16_t bertlv_get_tl_length(const uint8_t* tlv, uint32_t* value_len);

/**
 * @brief                 Calculate the size of TLV
 * @param[in] tlv         The pointer of TLV buffer
 * @return uint32_t       The size of TLV
 * @note
 */
uint32_t bertlv_get_tlv_length(const uint8_t* tlv);

/**
 * @brief                 Get integer value from a integer tlv. Maximum support 4 byte value
 * @param[in] tlv         Pointer of integer TLV
 * @param[out] tlv_size   Size of integer TLV
 * @return uint32_t       The integer value
 * @note                  If you do not confirm that the length is less than or equal to 4, please do not use this method
 */
uint32_t bertlv_get_integer(const uint8_t* tlv, uint32_t* tlv_size);

/**
 * @brief                 Get the next TLV from current TLV
 * @param[in] tlv         Pointer of Current TLV
 * @param[out] next       Pointer of next TLV 
 * @return uint8_t <br>
 *   <em> 0 </em>    ----success    <br>
 *   <em> -1 </em>   ----failed.
 * @note                  
 */
uint8_t bertlv_move_to_next(const uint8_t* tlv, uint8_t** next);

/**
 * @brief                 Build a BER-TLV to special buffer
 * @param[in] tag         The tag
 * @param[in] len         The length
 * @param[in] value       The pointer of value buffer
 * @param[out] dst        The buffer to store TLV data
 * @return uint32_t       The size of TLV
 * @note                  Support for overlapping value and dst arrays
 */
uint32_t bertlv_build_tlv(uint16_t tag, uint32_t len, const uint8_t* value, uint8_t* dst);

/**
 * @brief                 Build a BER-TLV to special buffer, with integer value
 * @param[in] tag         The tag
 * @param[in] integer     The integer value
 * @param[out] dst        The buffer to store TLV data
 * @return uint32_t       The size of TLV
 * @note                  
 */
uint32_t bertlv_build_integer_tlv(uint16_t tag, uint32_t integer, uint8_t* dst);

#ifdef _cplusplus
}
#endif

#endif
 
 