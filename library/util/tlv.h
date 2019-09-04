
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : md5.h
 * Date        : 2017.09.01
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __TLV_H__
#define __TLV_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

typedef struct TLV {
    uint8_t tag;
    uint16_t offset;
    uint16_t length;
} tlv_t;

void init_tlv(void);
uint16_t get_cur_tlv_tag(void);
uint16_t get_cur_tlv_off(void);
uint16_t get_cur_tlv_len(void);
uint32_t get_length(uint8_t *baBuffer, uint8_t mode);
uint32_t set_length(uint8_t *baBuffer, uint32_t sLength);
uint8_t *get_value_buffer(uint8_t *baBuffer);
uint8_t *get_simple_tag_tlv(uint16_t tag, uint8_t *baBuffer, uint32_t sLength, uint16_t sOccurence);
void set_cur_tlv_tag(uint8_t tag);
void set_cur_tlv_off(uint16_t off);
void set_cur_tlv_len(uint16_t len);
void append_tl_byteV(uint8_t *buffer, uint8_t tag, uint8_t val);
void append_tl_shortV(uint8_t *buffer, uint8_t tag, uint16_t val);
void append_tl_bufferV(uint8_t *buffer, uint8_t tag, uint8_t *valBuf, uint8_t valOff, uint8_t valLen);
uint8_t get_byte(uint8_t *buf);
uint16_t get_short(uint8_t *buf);

/* return the opinter buffer of the tag-value buffer */
uint8_t *get_simple_tlv(uint16_t tag, uint8_t *buffer, uint32_t len, uint32_t *tag_len, uint32_t *left_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __TLV_H__
