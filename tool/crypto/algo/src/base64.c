
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : base64.c
 * Date        : 2018.08.30
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include "base64.h"
#include <stdbool.h>
#include <stdio.h>

#define   RT_ERROR          -1
#define   RT_SUCCESS         0

int bytes2hexstring(const uint8_t *bytes, uint16_t length, char *hextring)
{
    uint16_t i = 0, cnt = 0;
    byte_t tmp;

    if ((bytes == NULL) || (hextring == NULL)) {
        //MSG_PRINTF(LOG_ERR, "INVALID PARAMETER\n\n");
        return RT_ERROR;
    }

    while (i < length) {
        tmp = *(byte_t * ) & bytes[i++];
        if (tmp.high <= 9)
            hextring[cnt++] = tmp.high + '0';
        else
            hextring[cnt++] = tmp.high - 10 + 'A';

        if (tmp.low <= 9)
            hextring[cnt++] = tmp.low + '0';
        else
            hextring[cnt++] = tmp.low - 10 + 'A';
    }

    hextring[cnt] = '\0';

    return RT_SUCCESS;
}

static uint8_t get_index_from_char(char c)
{
    if ((c >= 'A') && (c <= 'Z'))           return (c - 'A');
    else if ((c >= 'a') && (c <= 'z'))      return (c - 'a' + 26);
    else if ((c >= '0') && (c <= '9'))      return (c - '0' + 52);
    else if (c == '+')                      return 62;
    else if (c == '/')                      return 63;
    else if (c == '=')                      return 64;
    else if ((c == '\r') || (c == '\n'))    return 254;
    else                                    return 255;
}

static char get_char_from_index(uint8_t i)
{
    if ((i >= 0) && (i <= 25))              return (i + 'A');
    else if ((i >= 26) && (i <= 51))        return (i - 26 + 'a');
    else if ((i >= 52) && (i <= 61))        return (i - 52 + '0');
    else if (i == 62)                       return '+';
    else if (i == 63)                       return '/';
    else                                    return '=';
}

int rt_base64_encode(const uint8_t *in, uint16_t in_len, char *out)
{
    int i;
    uint32_t tmp = 0;
    uint16_t out_len = 0;
    uint16_t left = in_len;

    if ((in == NULL) || (out == NULL)) {
        return RT_ERROR;
    }

    for (i = 0; i < in_len;) {
        if (left >= 3) {
            tmp = in[i];
            tmp = (tmp << 8) + in[i+1];
            tmp = (tmp << 8) + in[i+2];
            out[out_len++] = get_char_from_index((tmp & 0x00FC0000) >> 18);
            out[out_len++] = get_char_from_index((tmp & 0x0003F000) >> 12);
            out[out_len++] = get_char_from_index((tmp & 0x00000FC0) >> 6);
            out[out_len++] = get_char_from_index(tmp & 0x0000003F);
            left -= 3;
            i += 3;
        } else {
            break;
        }
    }

    if (left == 2) {
        tmp = in[i];
        tmp = (tmp << 8) + in[i+1];
        out[out_len++] = get_char_from_index((tmp & 0x0000FC00) >> 10);
        out[out_len++] = get_char_from_index((tmp & 0x000003F0) >> 4);
        out[out_len++] = get_char_from_index((tmp & 0x0000000F) << 2);
        out[out_len++] = get_char_from_index(64);
    } else if (left == 1) {
        tmp = in[i];
        out[out_len++] = get_char_from_index((tmp & 0x000000FC) >> 2);
        out[out_len++] = get_char_from_index((tmp & 0x00000003) << 4);
        out[out_len++] = get_char_from_index(64);
        out[out_len++] = get_char_from_index(64);
    }

    out[out_len] = '\0';

    return RT_SUCCESS;
}

int rt_base64_decode(const char *in, uint8_t *out, uint16_t *out_len)
{
    uint16_t i = 0, cnt = 0;
    uint8_t c, in_data_cnt;
    bool error_msg = false;
    uint32_t tmp = 0;

    if ((in == NULL) || (out == NULL) || (out_len == NULL)) {
        //MSG_PRINTF(LOG_ERR, "INVALID PARAMETERS\n");
        return RT_ERROR;
    }

    in_data_cnt = 0;
    while (in[i] != '\0') {
        c = get_index_from_char(in[i++]);
        if (c == 255) {
            //MSG_PRINTF(LOG_ERR, "INVALID MESSAGE CODE\n");
            return RT_ERR_BASE64_BAD_MSG;
        } else if (c == 254) {
            continue;           // Carriage return or newline feed, skip
        } else if (c == 64) {
            break;              // Meet '=', break
        }

        // No comments needed if you know about BASE64
        tmp = (tmp << 6) | c;
        if (++in_data_cnt == 4) {
            out[cnt++] = (uint8_t)((tmp >> 16) & 0xFF);
            out[cnt++] = (uint8_t)((tmp >> 8) & 0xFF);
            out[cnt++] = (uint8_t)(tmp & 0xFF);
            in_data_cnt = 0;
            tmp = 0;
        }
    }

    // Meet '=' or '\0'
    if (in_data_cnt == 3) {          // 3 chars before '=', encoded msg like xxx= OR
        tmp = (tmp << 6);           // 3 chars before '\0', encoded msg like xxx, considered '=' omitted
        out[cnt++] = (uint8_t)((tmp >> 16) & 0xFF);
        out[cnt++] = (uint8_t)((tmp >> 8) & 0xFF);
    } else if (in_data_cnt == 2) {   // 2 chars before '=', encoded msg like xx== OR
        tmp = (tmp << 6);           // 2 chars before '\0', encoded msg like xx, considered '=' omitted
        tmp = (tmp << 6);
        out[cnt++] = (uint8_t)((tmp >> 16) & 0xFF);
    } else if (in_data_cnt != 0) {
        error_msg = true;           // Warn that the message format is wrong, but we tried our best to decode
    }

    *out_len = cnt;

    return (error_msg ? 1 : 0);
}
