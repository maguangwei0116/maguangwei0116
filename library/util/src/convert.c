
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : convert.c
 * Date        : 2018.08.30
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <string.h>
#include "convert.h"

void swap_nibble(uint8_t *buf, uint16_t swap_cnt)
{
    uint16_t i;
    for (i = 0; i < swap_cnt; i++) {
        buf[i] = ((buf[i] & 0x0F) << 4) | ((buf[i] & 0xF0) >> 4);
    }
}

void pad_F(char *raw_string, char *target, uint8_t size)
{
    uint8_t i;
    for (i = strlen(raw_string); i < size; i++) {
        raw_string[i] = 'F';
    }
    raw_string[i] = '\0';
    strcpy(target, (const char *) raw_string);
    MSG_PRINTF(LOG_DBG, "raw_string[%d]: %s\n", i, raw_string);
    MSG_PRINTF(LOG_DBG, "target[%d]: %s\n", i, target);
}

int hexstring2bytes(const char *hextring, uint8_t *bytes, uint16_t *length)
{
    const char *p = hextring;
    byte_t tmp;
    uint16_t cnt = 0;

    if ((hextring == NULL) || (bytes == NULL) || (length == NULL)) {
        MSG_PRINTF(LOG_ERR, "INVALID PARAMETER\n\n");
        return RT_ERROR;
    }

    *length = 0;
    if ((strlen(p) % 2) != 0) {
        MSG_PRINTF(LOG_ERR, "INVALID STRING LENGTH\n\n");
        return RT_ERR_CONVER_BAD_LEN;
    }

    while (*p != '\0') {
        if ((*p >= '0') && (*p <= '9'))
            tmp.high = *p - '0';
        else if ((*p >= 'A') && (*p <= 'F'))
            tmp.high = *p - 'A' + 10;
        else if ((*p >= 'a') && (*p <= 'f'))
            tmp.high = *p - 'a' + 10;
        else {
            MSG_PRINTF(LOG_ERR, "INVALID HEXSTRING\n\n");
            return RT_ERR_CONVER_BAD_CHAR;
        }
        p++;
        if ((*p >= '0') && (*p <= '9'))
            tmp.low = *p - '0';
        else if ((*p >= 'A') && (*p <= 'F'))
            tmp.low = *p - 'A' + 10;
        else if ((*p >= 'a') && (*p <= 'f'))
            tmp.low = *p - 'a' + 10;
        else {
            MSG_PRINTF(LOG_ERR, "INVALID HEXSTRING\n\n");
            return RT_ERR_CONVER_BAD_CHAR;
        }
        p++;
        bytes[cnt++] = *(uint8_t * ) & tmp;
    }
    *length = cnt;
    return RT_SUCCESS;
}

int bytes2hexstring(const uint8_t *bytes, uint16_t length, char *hextring)
{
    uint16_t i = 0, cnt = 0;
    byte_t tmp;
    if ((bytes == NULL) || (hextring == NULL)) {
        MSG_PRINTF(LOG_ERR, "INVALID PARAMETER\n\n");
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

rt_bool strncpy_case_insensitive(int8_t *src,int8_t *obj,int16_t len)
{
    int i;
    for (i = 0; i < len; i++) {

        if (src[i] == obj[i] || src[i] == (obj[i] + 'a' - 'A') || obj[i] == src[i] + 'a' - 'A') {
            continue;
        } else {
            MSG_PRINTF(LOG_WARN, "no.%d number is not equal,src:%c,obj:%c\n", i, src[i], obj[i]);
            return RT_FALSE;
        }
    }
    return RT_TRUE;
}

static unsigned char to_ascii(unsigned char ch)
{
    if (ch <= 9) {
        ch += ('0' - 0);
    }
    else if (ch >= 0xa && ch <= 0xf) {
        ch += ('A' - 0xa);
    } else {
        ch = 'F';
    }
    return ch;
}

void bytestring_to_charstring(int8_t *bytestring,int8_t *charstring,int16_t length)
{
    int32_t i = 0;
    int8_t left,right;
    for (i = 0; i < length ; i ++) {
        left = to_ascii((bytestring[i] >> 4) & 0x0F);
        right = to_ascii(bytestring[i] & 0x0F);

        charstring[2 * i] = left;
        charstring[2 * i + 1] = right;
    }
    charstring[2*i] = '\0';
}



