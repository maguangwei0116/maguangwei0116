
/******************************************************************************
  @file    sha1.h
  @brief   The ecc include file.

  DESCRIPTION:
  This is the header file for code which implements the Secure
  Hashing Algorithm 1 as defined in FIPS PUB 180-1 published
  April 17, 1995.

  ---------------------------------------------------------------------------
  Copyright (c) 2018 Redtea Technologies, Inc. All Rights Reserved.
  Redtea Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#ifndef __SHA1_H__
#define __SHA1_H__

#include <stdint.h>

#define SHA1HashSize 20
/*
* If you do not have the ISO standard stdint.h header file, then you
* must typdef the following:
* name meaning
* uint32_t unsigned 32 bit integer
* uint8_t unsigned 8 bit integer (i.e., unsigned char)
* int_least16_t integer of >= 16 bits
*
*/

enum {
    shaSuccess = 0,
    shaNull,         /* Null pointer parameter */
    shaInputTooLong, /* input data too long */
    shaStateError    /* called Input after Result */
};

/*
* This structure will hold context information for the SHA-1
* hashing operation
*/
typedef struct SHA1Context {
    uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest */
    uint32_t Length_Low; /* Message length in bits */
    uint32_t Length_High; /* Message length in bits */
    /* Index into message block array */
    int_least16_t Message_Block_Index;
    uint8_t Message_Block[64]; /* 512-bit message blocks */
    int Computed; /* Is the digest computed  */
    int Corrupted; /* Is the message digest corrupted  */
} SHA1Context;

/*
* Function Prototypes
*/
int SHA1Reset( SHA1Context *);
int SHA1Input( SHA1Context *,const uint8_t *,unsigned int);
int SHA1Result( SHA1Context *,uint8_t Message_Digest[SHA1HashSize]);
void SHA1Calc_calculate (uint8_t* inPut, uint32_t inPutlength, uint8_t * outPut) ;

#endif  // __SHA1_H__
