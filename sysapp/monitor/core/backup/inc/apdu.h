
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : apdu.h
 * Date        : 2019.10.16
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __APDU_H__
#define __APDU_H__

#include "rt_type.h"

#define RT_ERR_UNKNOWN_ERROR                    -1
#define RT_ERR_APDU_STORE_DATA_FAIL             -203
#define RT_ERR_APDU_SEND_FAIL                   -204
#define RT_ERR_APDU_OPEN_CHANNEL_FAIL           -201

#define GET_HIGH_BYTE(x)                        ((x >> 8) & 0xFF)
#define GET_LOW_BYTE(x)                         (x & 0xFF)

// Normal processing
#define SW_NORMAL                               0x9000
#define SW_NORMAL_PENDING_PROACTIVE_COMMAND     0x9100

// Warnings
#define SW_WARNING                              0x6200
#define SW_DATA_CORRUPTED                       0x6281
#define SW_EOF_REACHED                          0x6282
#define SW_FILE_DEACTIVATED                     0x6283
#define SW_FILE_TERMINATED                      0x6285
#define SW_RETRY                                0x63C0

// Checking errors
#define SW_WRONG_LENGTH                         0x6700
#define SW_WRONG_PARAMS                         0x6B00
#define SW_INS_NOT_SUPPORTED                    0x6D00
#define SW_CLA_NOT_SUPPORTED                    0x6E00
#define SW_UNKNOWN_ERROR                        0x6F00

// Functions in CLA not supported
#define SW_CLASS_FUNCTION_NOT_SUPPORTED         0x6800
#define SW_LOGICAL_CHANNEL_NOT_SUPPORTED        0x6881
#define SW_SECURE_MESSAGING_NOT_SUPPORTED       0x6882

// Command not allowed
#define SW_COMMAND_NOT_ALLOWED                  0x6900
#define SW_INCOMPATIBLE_WITH_FILE               0x6981
#define SW_SECURITY_STATUS_NOT_SATISFIED        0x6982
#define SW_PIN_BLOCKED                          0x6983
#define SW_REF_DATA_INVALIDATED                 0x6984
#define SW_CONDITIONS_NOT_SATISFIED             0x6985
#define SW_NO_FILE_SELECTED                     0x6986
#define SW_APPLET_SELECT_FAILED                 0x6999

// Wrong parameters
#define SW_INCORRECT_DATA                       0x6A80
#define SW_FUNCTION_NOT_SUPPORTED               0x6A81
#define SW_FILE_NOT_FOUND                       0x6A82
#define SW_RECORD_NOT_FOUND                     0x6A83
#define SW_FILE_FULL                            0x6A84
#define SW_INCORRECT_P1_P2                      0x6A86
#define SW_INCONSISTENT_LC                      0x6A87
#define SW_REF_DATA_NOT_FOUND                   0x6A88

#define LPA_AT_BLOCK_BUF                        538

typedef struct APDU_COMMAND {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t lc;
    uint8_t data;
    uint8_t le;
} apdu_t;

typedef struct STATUS_WORD {
    uint8_t sw1;
    uint8_t sw2;
} sw_t;

typedef enum CHANNEL_OPERATION {
    OPEN_CHANNEL = 0,
    CLOSE_CHANNEL
} channel_opt_e;

int rt_open_channel(uint8_t *channel);
int rt_close_channel(uint8_t channel);
int cmd_store_data(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel);

#endif // __APDU_H__
