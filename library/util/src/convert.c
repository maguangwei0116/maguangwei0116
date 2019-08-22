
#include "convert.h"
#include <string.h>

void swap_nibble(uint8_t *buf, uint16_t swap_cnt)
{
    uint16_t i;
    for(i=0; i<swap_cnt; i++) {
        buf[i] = ((buf[i] & 0x0F) << 4) | ((buf[i] & 0xF0) >> 4);
    }
}

void pad_F(char *raw_string, char *target, uint8_t size)
{
    uint8_t i;
    for(i=strlen(raw_string); i<size; i++) {
        raw_string[i] = 'F';
    }
    raw_string[i] = '\0';
    strcpy(target, (const char *)raw_string);
    MSG_PRINTF(LOG_DBG, "raw_string[%d]: %s\n", i, raw_string);
    MSG_PRINTF(LOG_DBG, "target[%d]: %s\n", i, target);
}

int hexstring2bytes(const char *hextring, uint8_t *bytes, uint16_t *length)
{
    const char *p = hextring;
    byte_t tmp;
    uint16_t cnt = 0;

    if((hextring == NULL) || (bytes == NULL) || (length == NULL)) {
        MSG_PRINTF(LOG_ERR, "INVALID PARAMETER\n\n");
        return RT_ERROR;
    }

    *length = 0;
    if((strlen(p) % 2) != 0) {
        MSG_PRINTF(LOG_ERR, "INVALID STRING LENGTH\n\n");
        return RT_ERR_CONVER_BAD_LEN;
    }

    while(*p != '\0') {
        if((*p >= '0') && (*p <= '9'))
            tmp.high = *p - '0';
        else if((*p >= 'A') && (*p <= 'F'))
            tmp.high = *p - 'A' + 10;
        else if((*p >= 'a') && (*p <= 'f'))
            tmp.high = *p - 'a' + 10;
        else {
            MSG_PRINTF(LOG_ERR, "INVALID HEXSTRING\n\n");
            return RT_ERR_CONVER_BAD_CHAR;
        }
        p++;
        if((*p >= '0') && (*p <= '9'))
            tmp.low = *p - '0';
        else if((*p >= 'A') && (*p <= 'F'))
            tmp.low = *p - 'A' + 10;
        else if((*p >= 'a') && (*p <= 'f'))
            tmp.low = *p - 'a' + 10;
        else {
            MSG_PRINTF(LOG_ERR, "INVALID HEXSTRING\n\n");
            return RT_ERR_CONVER_BAD_CHAR;
        }
        p++;
        bytes[cnt++] = *(uint8_t *)&tmp;
    }
    *length = cnt;
    return RT_SUCCESS;
}

int bytes2hexstring(const uint8_t *bytes, uint16_t length, char *hextring)
{
    uint16_t i = 0, cnt = 0;
    byte_t tmp;
    if((bytes == NULL) || (hextring == NULL)) {
        MSG_PRINTF(LOG_ERR, "INVALID PARAMETER\n\n");
        return RT_ERROR;
    }

    while(i < length) {
        tmp = *(byte_t *)&bytes[i++];
        if(tmp.high <= 9)
            hextring[cnt++] = tmp.high + '0';
        else
            hextring[cnt++] = tmp.high - 10 + 'A';

        if(tmp.low <= 9)
            hextring[cnt++] = tmp.low + '0';
        else
            hextring[cnt++] = tmp.low - 10 + 'A';
    }

    hextring[cnt] = '\0';

    return RT_SUCCESS;
}
