
/******************************************************************************
  @file    crypto.c
  @brief   The crypto core file.

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2018 Redtea Technologies, Inc. All Rights Reserved.
  Redtea Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include "types.h"
#include "aes.h"
#include "ecc.h"
#include "sha256.h"

#define ECC_MODULE_LENGTH_BIT 256
#define ECC_MODULE_LENGTH 32
#define ECC_MODULE_ASC_LENGTH ECC_MODULE_LENGTH * 2

char mac_chain[32] = {0};
char ca_module[ECC_MODULE_ASC_LENGTH+1];
char ca_coef_a[ECC_MODULE_ASC_LENGTH+1];
char ca_coef_b[ECC_MODULE_ASC_LENGTH+1];
char ca_basepoint_x[ECC_MODULE_ASC_LENGTH+1];
char ca_basepoint_y[ECC_MODULE_ASC_LENGTH+1];
char ca_basepoint_order[ECC_MODULE_ASC_LENGTH+1];

const char curve_parameter[] = {
    0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,
    0x5A,0xC6,0x35,0xD8,0xAA,0x3A,0x93,0xE7,0xB3,0xEB,0xBD,0x55,0x76,0x98,0x86,0xBC,0x65,0x1D,0x06,0xB0,0xCC,0x53,0xB0,0xF6,0x3B,0xCE,0x3C,0x3E,0x27,0xD2,0x60,0x4B,
    0x6B,0x17,0xD1,0xF2,0xE1,0x2C,0x42,0x47,0xF8,0xBC,0xE6,0xE5,0x63,0xA4,0x40,0xF2,0x77,0x03,0x7D,0x81,0x2D,0xEB,0x33,0xA0,0xF4,0xA1,0x39,0x45,0xD8,0x98,0xC2,0x96,
    0x4F,0xE3,0x42,0xE2,0xFE,0x1A,0x7F,0x9B,0x8E,0xE7,0xEB,0x4A,0x7C,0x0F,0x9E,0x16,0x2B,0xCE,0x33,0x57,0x6B,0x31,0x5E,0xCE,0xCB,0xB6,0x40,0x68,0x37,0xBF,0x51,0xF5,
    0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBC,0xE6,0xFA,0xAD,0xA7,0x17,0x9E,0x84,0xF3,0xB9,0xCA,0xC2,0xFC,0x63,0x25,0x51
};

void init_curve_parameter(const char * curve)
{
    memset(ca_module, 0, sizeof(ca_module));
    memset(ca_coef_a, 0, sizeof(ca_coef_a));
    memset(ca_coef_b, 0, sizeof(ca_coef_b));
    memset(ca_basepoint_x, 0, sizeof(ca_basepoint_x));
    memset(ca_basepoint_y, 0, sizeof(ca_basepoint_y));
    memset(ca_basepoint_order, 0, sizeof(ca_basepoint_order));

    fnHexToAsc(ca_module,64,(uint8_t*)curve,ECC_MODULE_LENGTH);
    fnHexToAsc(ca_coef_a,64,(uint8_t*)(curve+ECC_MODULE_LENGTH),ECC_MODULE_LENGTH);
    fnHexToAsc(ca_coef_b,64,(uint8_t*)(curve+ECC_MODULE_LENGTH*2),ECC_MODULE_LENGTH);
    fnHexToAsc(ca_basepoint_x,64,(uint8_t*)(curve+ECC_MODULE_LENGTH*3),ECC_MODULE_LENGTH);
    fnHexToAsc(ca_basepoint_y,64,(uint8_t*)(curve+ECC_MODULE_LENGTH*4),ECC_MODULE_LENGTH);
    fnHexToAsc(ca_basepoint_order,64,(uint8_t*)(curve+ECC_MODULE_LENGTH*5),ECC_MODULE_LENGTH);
}
int generate_random(uint8_t * ptr_random_buf , int random_length)
{
    GenRandStr((char *)ptr_random_buf,random_length) ;
    //srand( (unsigned)time( NULL ) );
    return 0;
}

int ecc_gen_key_pair(char *ca_private_key , char *ca_public_key_x , char *ca_public_key_y)
{
    if( SetCurveParam( ca_module , ca_coef_a , ca_coef_b , ca_basepoint_x , ca_basepoint_y , ca_basepoint_order ) != 1 ) {
        return 0;
    }

    GenerateKeyPair( ca_private_key , ca_public_key_x , ca_public_key_y );

    if( (CheckPrivateKey( ca_private_key)!= 1) || (CheckPublicKey(ca_public_key_x , ca_public_key_y)!= 1) ) {
        LOGE("GenerateKeyPair error");
        return 0;
    }
    return 1;
}
int ecc_verify_signature(uint8_t *input, int input_len, const uint8_t *pubkey, int pubkey_len, uint8_t *sigBuff, int  sig_len)
{
    char ca_public_key_x[70];
    char ca_public_key_y[70];
    char car[70];
    char cas[70];

    memset(ca_public_key_x, 0, sizeof(ca_public_key_x));
    memset(ca_public_key_y, 0, sizeof(ca_public_key_y));
    memset(car, 0, sizeof(car));
    memset(cas, 0, sizeof(cas));

    if ( SetCurveParam( ca_module , ca_coef_a , ca_coef_b , ca_basepoint_x , ca_basepoint_y , ca_basepoint_order ) != 1 ) {
        LOGE("set curvcaPublicKeyYe parameter error");
        return 0;
    }

    //LOGI_STR("verify beginning...");
    memcpy(ca_public_key_x, (SZ_UBYTE1 *)pubkey, 64);
    memcpy(ca_public_key_y, (SZ_UBYTE1 *)pubkey + 64, 64);
    if ( CheckPublicKey( ca_public_key_x , ca_public_key_y ) != 1 ) {
        LOGE("Check pk failed\n");
        return 0;
    }
    //printByteString(input,input_len);
    memcpy(car, sigBuff, 64);
    memcpy(cas, sigBuff + 64, 64);
    if ( Verify( (char *)input , ca_public_key_x , ca_public_key_y , car , cas ) ) {
        LOGD("verify sucess");
        return 1;
    } else {
        LOGE("verify fail");
        return 0;
    }

}

int ecc_verify(uint8_t *input, int input_len, const uint8_t *pubkey, int pubkey_len, uint8_t *sigBuff, int  sig_len)
{
    char ca_hm[80];
    char ca_public_key_x[70];
    char ca_public_key_y[70];
    char car[70];
    char cas[70];
    uint8_t sha[32];

    memset(ca_hm, 0, sizeof(ca_hm));
    memset(ca_public_key_x, 0, sizeof(ca_public_key_x));
    memset(ca_public_key_y, 0, sizeof(ca_public_key_y));
    memset(car, 0, sizeof(car));
    memset(cas, 0, sizeof(cas));

    //printByteString(ca_module,64);
    //printByteString(ca_coef_a,64);
    //printByteString(ca_coef_b,64);
    //printByteString(ca_basepoint_x,64);
    //printByteString(ca_basepoint_y,64);
    //printByteString(ca_basepoint_order,64);

    if ( SetCurveParam( ca_module , ca_coef_a , ca_coef_b , ca_basepoint_x , ca_basepoint_y , ca_basepoint_order ) != 1 ) {
        LOGE("set curvcaPublicKeyYe parameter error");
        return 0;
    }
    //LOGI_STR("verify beginning...");
    fnHexToAsc(ca_public_key_x, 64, (SZ_UBYTE1 *)pubkey + 1, 32);
    fnHexToAsc(ca_public_key_y, 64, (SZ_UBYTE1 *)pubkey + 33, 32);
    if ( CheckPublicKey( ca_public_key_x , ca_public_key_y ) != 1 ) {
        LOGE("Check pk failed\n");
        return 0;
    }
    //printByteString(input,input_len);
    Sha256Calc_calculate(input , input_len, sha);
    fnHexToAsc(ca_hm, 64, sha, 32);
    fnHexToAsc(car, 64, sigBuff, 32);
    fnHexToAsc(cas, 64, sigBuff + 32, 32);
    if ( Verify( ca_hm , ca_public_key_x , ca_public_key_y , car , cas ) ) {
        LOGD("verify sucess");
        return 1;
    } else {
        LOGE("verify fail");
        return 0;
    }

}

int ecc_sign( uint8_t *input, int input_len, uint8_t *privkey, int privkey_len, uint8_t *output, int *output_len)
{
    char *pcak;
    char ca_hm[80];
    char ca_private_key[70];
    uint8_t sha[32] ;
    char car[70] ;
    char cas[70] ;

    memset(ca_hm, 0, sizeof(ca_hm));
    memset(ca_private_key, 0, sizeof(ca_private_key));

    if ( SetCurveParam( ca_module , ca_coef_a , ca_coef_b , ca_basepoint_x , ca_basepoint_y , ca_basepoint_order ) != 1 ) {
        LOGE("set curve parameter error");
        return 0;
    }
    fnHexToAsc(ca_private_key, 64, privkey, privkey_len);
    //printByteString(privkey,privkey_len);
    if ( CheckPrivateKey( ca_private_key ) != 1 ) {
        LOGE("check private Key error");
        return 0;
    }

    //pcak = cak;
    //if( strlen(cak) == 0x00 )
    //{
        pcak = NULL;
    //}

    memset(ca_hm, 0, sizeof(ca_hm));
    Sha256Calc_calculate(input ,input_len, sha);
    //printByteString(sha,32);
    fnHexToAsc(ca_hm, 64, sha, 32);
    if ( Sign(ca_hm , pcak , ca_private_key , car , cas ) != 1 ) {
        return 0;
    }

    fnAscToHex(output, 32, car, strlen(car));
    fnAscToHex(output + 32, 32, cas, strlen(cas));

    *output_len = 64 ;
    return 1;
}

int ecc_sign_hash( uint8_t *input, int input_len, uint8_t *privkey, int privkey_len, uint8_t *output, int *output_len)
{
    char *pcak;
    char car[70] ;
    char cas[70] ;

    if ( SetCurveParam( ca_module , ca_coef_a , ca_coef_b , ca_basepoint_x , ca_basepoint_y , ca_basepoint_order ) != 1 ) {
        LOGE("set curve parameter error");
        return 0;
    }
    //printByteString(privkey,privkey_len);
    if ( CheckPrivateKey( (char *)privkey ) != 1 ) {
        LOGE("check private Key error");
        return 0;
    }

    //pcak = cak;
    //if( strlen(cak) == 0x00 )
    //{
        pcak = NULL;
    //}
    if ( Sign((char *)input , pcak , (char *)privkey , car , cas ) != 1 ) {
        return 0;
    }

    fnAscToHex(output, 32, car, strlen(car));
    fnAscToHex(output + 32, 32, cas, strlen(cas));
    *output_len = 64 ;

    return 1;
}

int ecc_generate_key( uint8_t* outpub, int *outpub_len, uint8_t * outprv, int *outprv_len)
{
    char publicX[70] ;
    char publicY[70] ;
    char privateKey[70] ;

    memset(publicX, 0, strlen(publicX));
    memset(publicY, 0, strlen(publicY));
    memset(privateKey, 0, strlen(privateKey));
    int returnCode = ecc_gen_key_pair (privateKey, publicX, publicY);
    //printByteString(privateKey,64);
    if (returnCode != 0) {
        fnAscToHex(outpub,32, publicX,64);
        fnAscToHex(outpub+32,32, publicY,64);
        *outpub_len = 64 ;
        fnAscToHex(outprv,32,privateKey,64);
        *outprv_len = 32 ;

        LOGD("key generation ok");
        //printByteString(outpub,64);
        //printByteString(outprv,32);
    }
    return returnCode ;
}

int ecc_key_agreement(uint8_t*prvkey, int prvkey_len, uint8_t * pubkey, int pubkey_len, uint8_t* shsout, int *shsout_len)
{
    char cad_a[70];
    char ca_pb_x[70];
    char ca_pb_y[70];
    char ca_z_ab[70];

    memset(cad_a, 0, sizeof(cad_a));
    memset(ca_pb_x, 0, sizeof(ca_pb_x));
    memset(ca_pb_y, 0, sizeof(ca_pb_y));
    memset(ca_z_ab, 0, sizeof(ca_z_ab));

    //LOGI_STR("key agrreement begin...") ;
    if ( SetCurveParam( ca_module , ca_coef_a , ca_coef_b , ca_basepoint_x , ca_basepoint_y , ca_basepoint_order ) != 1 ) {
        LOGE("key curve parameter error...");
        return 0;
    }
    fnHexToAsc(cad_a,64,prvkey,prvkey_len);
    if ( CheckPrivateKey( cad_a ) != 1 ) {
        LOGE("key private key error...");
        return 0;
    }

    fnHexToAsc(ca_pb_x,64,pubkey+1,32);
    fnHexToAsc(ca_pb_y,64,pubkey+33,32);
    if ( CheckPublicKey( ca_pb_x , ca_pb_y ) != 1 ) {
        LOGE("key public key error...");
        return 0;
    }

    if ( KeyAgreement( cad_a , ca_pb_x , ca_pb_y , ca_z_ab ) != 1 ) {
        LOGE("key agrreement error...");
        return 0;
    } else {
        LOGD("key agrreement sucess...");
    }

    fnAscToHex(shsout,32,ca_z_ab,64);
    *shsout_len = 32 ;
    return 1;

}


int sha256_cos(uint8_t* input, int input_len, uint8_t *output, int * output_len)
{
    Sha256Calc_calculate (input, input_len, output);
    return 1 ;
}

int aes_sign(uint8_t*input, int input_len, uint8_t* key, uint8_t* output, int *output_len)
{
    if (AES_Init(key, 128)!=0) {
        LOGE("AES init key error...");
        return 0;
    }
    memset(output,0,16);
    AES_CMAC(input,input_len,output,AES_CMAC_DOFINAL_MODE);
    *output_len = 8 ;
    return 1;
}
int aes_enc(uint8_t *input, int input_len, uint8_t *key, uint8_t * icv, uint8_t *output, int *output_len, int mode)
{
    AES_Init(key,128);
    if (AES_Encrypt(input,output,input_len,icv,mode) != 0) {
        LOGE("error occurrent...");
        return 1 ;
    }
    *output_len = input_len ;
    return 0;
}
int aes_dec(uint8_t *input, int input_len, uint8_t *key, uint8_t * icv, uint8_t *output, int *output_len, int mode)
{
    AES_Init(key,128);
    if (AES_Decrypt(input,output,input_len,icv,mode)) {
        LOGE("error occurrent...");
        return 1;
    }
    *output_len = input_len ;
    return 0;
}
