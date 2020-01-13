
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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "rt_type.h"

#define SHA256_BLOCK_SIZE   32

typedef struct SHA256_CTX {
    uint8_t                 data[64];
    uint32_t                datalen;
    uint64_t                bitlen;
    uint32_t                state[8];
} sha256_ctx_t;

extern void sha256_init(sha256_ctx_t *ctx);
extern void sha256_update(sha256_ctx_t *ctx, const uint8_t data[], int32_t len);
extern void sha256_final(sha256_ctx_t *ctx, uint8_t hash[]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __HASH_H__ */

