

/******************************************************************************
  @file    crypto.h
  @brief   The crypto include file.

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2018 Redtea Technologies, Inc. All Rights Reserved.
  Redtea Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <stdint.h>

void init_curve_parameter(const char *curve);
int generate_random(uint8_t *ptrRandomBuf , int randomLength);
int ecc_gen_key_pair(char *caPrivateKey , char *caPublicKeyX , char *caPublicKeyY);
int ecc_verify(uint8_t *input, int input_len, const uint8_t *pubkey, int pubkey_len, uint8_t *sigBuff, int  sig_len);
int ecc_sign( uint8_t *input, int input_len, uint8_t* privkey, int privkey_len, uint8_t *output, int *output_len);
int ecc_generate_key( uint8_t *outpub, int *outpub_len, uint8_t *outprv, int *outprv_len);
int ecc_key_agreement(uint8_t *prvkey, int prvkey_len, uint8_t * pubkey, int pubkey_len, uint8_t *shsout, int *shsout_len);
int sha256_cos(uint8_t *input, int input_len, uint8_t *output, int * output_len);
int aes_sign(uint8_t *input, int input_len, uint8_t* key, uint8_t *output, int *output_len);
int aes_enc(uint8_t *input, int input_len, uint8_t *key, uint8_t *icv, uint8_t *output, int *output_len, int mode);
int aes_dec(uint8_t *input, int input_len, uint8_t *key, uint8_t *icv, uint8_t *output, int *output_len, int mode);
int ecc_sign_hash( uint8_t *input, int input_len, uint8_t *privkey, int privkey_len, uint8_t *output, int *output_len);
int ecc_verify_signature(uint8_t *input, int input_len, const uint8_t *pubkey, int pubkey_len, uint8_t *sigBuff, int  sig_len);

#endif  // __CRYPTO_H__
