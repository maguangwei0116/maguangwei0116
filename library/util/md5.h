
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

#ifndef __MD5_H__
#define __MD5_H__

#include "rt_type.h"

typedef struct MD5_CTX {
    uint32_t count[2];
    uint32_t state[4];
    uint8_t  buffer[64];
} MD5_CTX, md5;

#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))

#define MD5_STRING_LENGTH     32

#define FF(a,b,c,d,x,s,ac) \
{ \
        a += F(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b; \
}
#define GG(a,b,c,d,x,s,ac) \
{ \
        a += G(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b; \
}
#define HH(a,b,c,d,x,s,ac) \
{ \
        a += H(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b; \
}
#define II(a,b,c,d,x,s,ac) \
{ \
        a += I(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b; \
}

void MD5Init(MD5_CTX *context);
void MD5Transform(unsigned int state[4], unsigned char block[64]);
void MD5Update(MD5_CTX *context, unsigned char *input, unsigned int inputlen);
void MD5Final(MD5_CTX *context, unsigned char digest[16]);
void get_md5_string(int8_t *input, int8_t *output);
void get_ascii_string(uint8_t *input ,uint16_t len, uint8_t *output);
void get_ascii_data(uint8_t *input, uint16_t len, uint8_t *output);

#endif /*__MD5_H__*/
