
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : tlv.c
 * Date        : 2018.08.30
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <string.h>

#include "tlv.h"
#include "rt_os.h"

#ifndef COS_MEMSET
#define COS_MEMSET(s, c, n)    rt_os_memset(s, c, n)
#endif

#ifndef COS_MEMCPY
#define COS_MEMCPY(d, s, n)    rt_os_memcpy(d, s, n)
#endif

static tlv_t g_tlv;

void init_tlv(void)
{
    COS_MEMSET((uint8_t * ) & g_tlv, 0, sizeof(g_tlv));
}

void free_tlv(void)
{
}

uint16_t get_cur_tlv_tag(void)
{
    return g_tlv.tag;
}

uint16_t get_cur_tlv_off(void)
{
    return g_tlv.offset;
}

uint16_t get_cur_tlv_len(void)
{
    return g_tlv.length;
}

void set_cur_tlv_tag(uint8_t tag)
{
    g_tlv.tag = tag;
}

void set_cur_tlv_off(uint16_t off)
{
    g_tlv.offset = off;
}

void set_cur_tlv_len(uint16_t len)
{
    g_tlv.length = len;
}

void append_tl_byte_v(uint8_t *buffer, uint8_t tag, uint8_t val)
{
    uint16_t curr_len;
    uint16_t curr_off;

    curr_len = get_cur_tlv_len();
    curr_off = get_cur_tlv_off();
    curr_len += 3;
    buffer[curr_off++] = tag;
    buffer[curr_off++] = 1;
    buffer[curr_off++] = val;
    set_cur_tlv_len(curr_len);
    set_cur_tlv_off(curr_off);
}

void append_tl_short_v(uint8_t *buffer, uint8_t tag, uint16_t val)
{
    uint16_t curr_len = get_cur_tlv_len(), curr_off = get_cur_tlv_off();
    curr_len += 4;
    buffer[curr_off++] = tag;
    buffer[curr_off++] = 2;
    buffer[curr_off++] = val >> 8;
    buffer[curr_off++] = val & 0xFF;
    set_cur_tlv_len(curr_len);
    set_cur_tlv_off(curr_off);
}

void append_tl_buffer_v(uint8_t *buffer, uint8_t tag, uint8_t *valBuf, uint8_t valOff, uint8_t valLen)
{
    uint16_t curr_len = get_cur_tlv_len(), curr_off = get_cur_tlv_off();
    curr_len += (valLen + 2);
    buffer[curr_off++] = tag;
    buffer[curr_off++] = valLen;
    COS_MEMCPY(buffer + curr_off, valBuf + valOff, valLen);
    curr_off += valLen;
    set_cur_tlv_len(curr_len);
    set_cur_tlv_off(curr_off);
}

uint32_t get_length(uint8_t *ba_buffer, uint8_t mode)
{
    uint32_t value;
    uint8_t length;
    uint8_t offset = 1;

    if ((ba_buffer[0] & 0x1F) == 0x1F) {
        offset++;
    }
    if (ba_buffer[offset] == 0x84) {
        value = ba_buffer[offset + 1];
        value = (uint32_t)((value << 8) | ba_buffer[offset + 2]);
        value = (uint32_t)((value << 8) | ba_buffer[offset + 3]);
        value = (uint32_t)((value << 8) | ba_buffer[offset + 4]);
        length = 5 + offset;
    } else if (ba_buffer[offset] == 0x83) {
        value = ba_buffer[offset + 1];
        value = (uint32_t)((value << 8) | ba_buffer[offset + 2]);
        value = (uint32_t)((value << 8) | ba_buffer[offset + 3]);
        length = 4 + offset;
    } else if (ba_buffer[offset] == 0x82) {
        value = ba_buffer[offset + 1];
        value = (uint32_t)((value << 8) | ba_buffer[offset + 2]);
        length = 3 + offset;
    } else if (ba_buffer[offset] == 0x81) {
        value = ba_buffer[offset + 1];
        length = 2 + offset;
    } else {
        value = ba_buffer[offset];
        length = 1 + offset;
    }
    if (mode == 1) {
        return length;
    }
    return value;
}

void copy_buffer(uint8_t *p_dst, uint8_t *p_src, uint32_t len)
{
    // Compare source address and destination address to know
    // the order (forward or backward copy)
    if (p_src > p_dst) {
        // forward copy
        while (len--) {
            *(p_dst++) = *(p_src++);
        }
    } else {
        p_src += len;
        p_dst += len;
        // backward copy
        while (len--) {
            *(--p_dst) = *(--p_src);
        }
    }
}

// current offset is TAG offset.
uint32_t set_length(uint8_t *ba_buffer, uint32_t s_length)
{
    uint8_t pad_number = 0;
    uint8_t s_offset = 1;

    if ((ba_buffer[0] & 0x1F) == 0x1F) {
        s_offset++;
    }
    if (s_length >= 0x100) {
        COS_MEMCPY(ba_buffer + s_offset + 3, ba_buffer + s_offset + 1, s_length);

        ba_buffer[s_offset] = 0x82;
        ba_buffer[s_offset + 1] = (uint8_t)(s_length >> 8);
        ba_buffer[s_offset + 2] = (uint8_t)(s_length);
        pad_number = 2;
    } else if (s_length > 0x7F) {
        COS_MEMCPY(ba_buffer + s_offset + 2, ba_buffer + s_offset + 1, s_length);
        ba_buffer[s_offset] = 0x81;
        ba_buffer[s_offset + 1] = (uint8_t) s_length;
        pad_number = 1;
    } else {
        ba_buffer[s_offset] = (uint8_t) s_length;
    }
    //return TLV length
    return (uint32_t)(s_length + s_offset + pad_number + 1);
}

uint8_t *get_value_buffer(uint8_t *ba_buffer)
{
    uint8_t s_offset = 1;

    if ((ba_buffer[0] & 0x1F) == 0x1F) {
        s_offset++;
    }
    if (ba_buffer[s_offset] == 0x82) {
        return (uint8_t * )(ba_buffer + 3 + s_offset);
    } else if (ba_buffer[s_offset] == 0x81) {
        return (uint8_t * )(ba_buffer + 2 + s_offset);
    } else {
        return (uint8_t * )(ba_buffer + 1 + s_offset);
    }
}

uint8_t *get_simple_tag_tlv(uint16_t tag, uint8_t *ba_buffer, uint32_t s_length, uint16_t sOccurence)
{
    uint16_t check_tag = 0;
    uint16_t counter = 0;
    uint8_t *the_end = (uint8_t * )(ba_buffer + s_length);

    while (ba_buffer < the_end) {
        check_tag = ba_buffer[0];
        //printf("0ck-tag=%04X\r\n", check_tag);
        if ((check_tag & 0x1F) == 0x1F) {
            check_tag <<= 8;
            check_tag |= ba_buffer[1];
        }
        //printf("1ck-tag=%04X\r\n", check_tag);
        if (check_tag == tag) {
            counter++;
            if (counter == sOccurence) {
                return ba_buffer;
            }
        }
        ba_buffer = (uint8_t * )(get_value_buffer(ba_buffer) + get_length(ba_buffer, 0));
    }
    return NULL;
}

uint8_t get_byte(uint8_t *buf)
{
    return buf[0];
}

uint16_t get_short(uint8_t *buf)
{
    return (buf[0] << 8) + buf[1];
}

/* return the opinter buffer of the tag-value buffer */
uint8_t *get_simple_tlv(uint16_t tag, uint8_t *buffer, uint32_t len, uint32_t *tag_len, uint32_t *left_len)
{
    uint8_t *p = NULL;

    p = get_simple_tag_tlv(tag, buffer, len, 1);
    if (p) {
        *tag_len = get_length(p, 0);
        *left_len = len - get_length(p, 1);
        //printf("==> tag: %X, p[0]=%02X, tag_len=%d, len_in=%d, left_len=%d\r\n", tag, p[0], *tag_len, len, *left_len);
        p = get_value_buffer(p);
    }

    return p;
}
