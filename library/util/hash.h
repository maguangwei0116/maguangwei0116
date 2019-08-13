
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : hash.h
 * Date        : 2019.01.03
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/
#ifndef __HASH_H__
#define __HASH_H__

#include "rt_type.h"

#define SHA256_BLOCK_SIZE                   32

typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} sha256_ctx;

void sha256_init(sha256_ctx *ctx);
void sha256_update(sha256_ctx *ctx, const uint8_t data[], int32_t len);
void sha256_final(sha256_ctx *ctx, uint8_t hash[]);

#endif // __HASH_H__
