
/******************************************************************************
  @file    sha256.h
  @brief   The ecc include file.

  DESCRIPTION:

  ---------------------------------------------------------------------------
  Copyright (c) 2018 Redtea Technologies, Inc. All Rights Reserved.
  Redtea Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#ifndef __SHA256_H__
#define __SHA256_H__

typedef unsigned char SZ_UBYTE1;
typedef unsigned short SZ_UBYTE2;
typedef signed char SZ_BYTE1;
typedef signed short SZ_BYTE2;
typedef float SZ_FLOAT;
typedef double SZ_DOUBLE;
typedef unsigned char SZ_UCHAR;
typedef signed char SZ_CHAR;
typedef unsigned short SZ_USHORT;
typedef signed short SZ_SHORT;
typedef signed short SZ_WCHAR;
typedef unsigned short SZ_UWCHAR;

typedef unsigned int SZ_UBYTE4;
typedef signed int SZ_BYTE4;
typedef unsigned long  SZ_UBYTE8;
typedef signed long SZ_BYTE8;

typedef unsigned int SZ_UINT;
typedef signed int SZ_INT;
typedef unsigned long SZ_ULONG;
typedef signed long SZ_LONG;

typedef unsigned int SZ_UPTR;
typedef signed int SZ_PTR;
typedef unsigned long SZ_UT;
typedef signed long SZ_T;


typedef struct _Sha256Calc {
    SZ_CHAR  Value[ 32 ];
    SZ_INT  DwordBufBytes;
    SZ_INT  ByteNumLo;
    SZ_INT  ByteNumHi;
    SZ_INT  reg[ 8 ]; /** h0 to h 7 -- old value store*/
    SZ_INT  DwordBuf[ 16 ]; /** data store */
    SZ_INT  Padding[ 64 ];
} Sha256Calc;

    //static const SZ_INT Sha256Calc_k[ 64 ];
    //static const SZ_UBYTE1 asc2hextable[256];
    //static const char hex2asctable[256];
SZ_T Sha256Calc_calculate(SZ_UBYTE1 *pIn, SZ_T inLen, SZ_UBYTE1 *pOut);
SZ_T Sha256Calc_calculate_Core( Sha256Calc* t, SZ_CHAR* dp, SZ_T dl );
SZ_T Sha256Calc_reset( Sha256Calc* t );
SZ_T Sha256Calc_calcBlock( SZ_INT* dp, SZ_INT* rp );
SZ_T  Sha256Calc_init( Sha256Calc* t );
SZ_T fnHexToAsc(char * Asc, int len1, SZ_UBYTE1 *Hex, int len2);
SZ_T fnAscToHex(SZ_UBYTE1 * Hex, int len1, char *Asc, int len2);

#endif  //__SHA256_H__
