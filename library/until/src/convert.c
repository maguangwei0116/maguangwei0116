#include "convert.h"


int8_t to_ascii(int8_t ch)
{
    if (ch <= 9) {
        ch += ('0' - 0);
    } else if (ch >= 0xA && ch <= 0xF) {
        ch += ('A' - 0xA);
    } else {
        MSG_WARN("can not contert to a ascii,ch = %02x", (uint8_t)ch);
        ch = 'F';
    }
    return ch;
}

rt_bool strncpy_case_insensitive(int8_t *src,int8_t *obj,int16_t len)
{
    int i;
    for (i = 0; i < len; i++) {

        if (src[i] == obj[i] || src[i] == (obj[i] + 'a' - 'A') || obj[i] == src[i] + 'a' - 'A') {
            continue;
        } else {
            MSG_WARN("no.%d number is not equal,src:%c,obj:%c\n", i, src[i], obj[i]);
            return RT_FALSE;
        }
    }
    return RT_TRUE;
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
