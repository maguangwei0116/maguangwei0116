
/******************************************************************************
  @file    aes.h
  @brief   The aes include file.

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2018 Redtea Technologies, Inc. All Rights Reserved.
  Redtea Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#ifndef __AES_H__
#define __AES_H__

#define AES_BITS_128        128
#define AES_BITS_192        192
#define AES_BITS_256        256

#define AES_ENCRYPT     0
#define AES_DECRYPT     1
#define AES_CBC     0
#define AES_ECB     1

// aes type
#define AES_ENCRYPT_CBC     0x00
#define AES_ENCRYPT_ECB     0x01
#define AES_DECRYPT_CBC     0x02
#define AES_DECRYPT_ECB     0x03


#define AES_MASK_CMAC       0x08
#define AES_MASK_EN_DE      0x02
#define AES_MASK_CBCECB     0x01

#define AES_CMAC_APPEND_MODE        0x00
#define AES_CMAC_DOFINAL_MODE       0x01

unsigned char AES_Init(const void *pKey, unsigned int bits);

unsigned char AES_Encrypt(unsigned char *pPlainText, unsigned char *pCipherText,
                 unsigned int nDataLen, unsigned char *pIV, unsigned char aes_mode);

unsigned char AES_Decrypt(unsigned char *pPlainText, unsigned char *pCipherText,
                 unsigned int nDataLen, unsigned char *pIV, unsigned int aes_mode);

unsigned char UHL_AES_BLOCK(unsigned char bMode, unsigned char  * pbDataIn, unsigned char  * pbDataOut);

void AES_CMAC(unsigned char *input, int length, unsigned char *cmac, int stage);

#endif  // __AES_H__

