#ifndef __BASE64_H__
#define __BASE64_H__

#include <rt_type.h>

#define RT_ERR_BASE64_BAD_MSG                   -2002

int rt_base64_encode(const uint8_t *in, uint16_t in_len, char *out);
int rt_base64_decode(const char *in, uint8_t *out, uint16_t *out_len);

#endif  //__BASE64_H__
