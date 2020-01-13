
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
    uint8_t     tag;
    uint16_t    offset;
    uint16_t    length;
} tlv_t;

extern void     init_tlv(void);
extern uint16_t get_cur_tlv_tag(void);
extern uint16_t get_cur_tlv_off(void);
extern uint16_t get_cur_tlv_len(void);
extern uint32_t get_length(uint8_t *baBuffer, uint8_t mode);
extern uint32_t set_length(uint8_t *baBuffer, uint32_t sLength);
extern uint8_t *get_value_buffer(uint8_t *baBuffer);
extern uint8_t *get_simple_tag_tlv(uint16_t tag, uint8_t *baBuffer, uint32_t sLength, uint16_t sOccurence);
extern void     set_cur_tlv_tag(uint8_t tag);
extern void     set_cur_tlv_off(uint16_t off);
extern void     set_cur_tlv_len(uint16_t len);
extern void     append_tl_byteV(uint8_t *buffer, uint8_t tag, uint8_t val);
extern void     append_tl_shortV(uint8_t *buffer, uint8_t tag, uint16_t val);
extern void     append_tl_bufferV(uint8_t *buffer, uint8_t tag, uint8_t *valBuf, uint8_t valOff, uint8_t valLen);
extern uint8_t  get_byte(uint8_t *buf);
extern uint16_t get_short(uint8_t *buf);

/* return the opinter buffer of the tag-value buffer */
extern uint8_t *get_simple_tlv(uint16_t tag, uint8_t *buffer, uint32_t len, uint32_t *tag_len, uint32_t *left_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TLV_H__ */

