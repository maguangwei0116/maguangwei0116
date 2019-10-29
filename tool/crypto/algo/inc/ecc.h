
/******************************************************************************
  @file    ecc.h
  @brief   The ecc include file.

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2018 Redtea Technologies, Inc. All Rights Reserved.
  Redtea Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#ifndef __ECC_H__
#define __ECC_H__

#include "tommath.h"

#define MAX_CHAR_LEN    0x0104

extern int m_modLen;
//static const char cTable[16];
extern mp_int m_Module,m_CoefA,m_CoefB,m_BasepointX,m_BasepointY,m_BasepointOrder;

typedef unsigned char bool;

bool CheckPrivateKey( char *pPrivateKey );
bool CheckPublicKey( char * pPublicKeyx , char * pPublicKeyy );
bool GenerateKeyPair( char *pPrivateKey , char *pPublicKeyX , char *pPublicKeyY );
bool GeneratePublicKey( char *pPrivateKey , char *pPublicKeyX , char *pPublicKeyY );
bool Sign( char *pHash , char *pk , char *pPrivateKey , char *pr , char *ps );
bool Verify( char *pHash , char *pPublicKeyX , char *pPublicKeyY , char *pr , char *ps );
bool KeyAgreement( char *pdA , char *pPBx , char *pPBy , char *pZab );
bool SetCurveParam( char *pModule , char *pCoefA , char *pCoefB , char *pBasepointX , char *pBasepointY , char *pBasepointOrder );
bool Encrypt( char *pPlain , char *pPublicKeyX , char *pPublicKeyY , char *pk , char *pCm , char *px1 , char *py1 );
bool Decrypt( char *pPrivateKey , char *pCm , char *px1 , char *py1 , char *pPlain );
bool CheckCurveParameter( void );
void GenRandStr( char *pRand , int len);
bool CheckKey(mp_int*QX,mp_int*QY,mp_int*sk,mp_int*n,mp_int*A,mp_int*B,mp_int*P);
void Mul2points(mp_int *qx,mp_int *qy, mp_int *px, mp_int *py,mp_int *d,mp_int *a,mp_int *p);
void Add2points(mp_int *x1,mp_int *y1,mp_int *x2,mp_int *y2,mp_int *x3,mp_int *y3,mp_int *a,mp_int *p);

#endif  // __ECC_H__
