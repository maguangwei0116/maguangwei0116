
#ifndef __LPA_ERROR_CODES_H__
#define __LPA_ERROR_CODES_H__

#define RT_SUCCESS                              0
#define RT_ERR_UNKNOWN_ERROR                    -1
#define RT_ERR_NULL_POINTER                     -2

#define RT_ERR_AT_OPEN_FAIL                     -100
#define RT_ERR_AT_READ_FAIL                     -101
#define RT_ERR_AT_WRITE_FAIL                    -102
#define RT_ERR_AT_SELECT_FAIL                   -103
#define RT_ERR_AT_TIME_OUT                      -104
#define RT_ERR_AT_WRONG_RSP                     -105

#define RT_ERR_APDU_CHANNEL_NOT_OPENED          -200
#define RT_ERR_APDU_OPEN_CHANNEL_FAIL           -201
#define RT_ERR_APDU_SELECT_AID_FAIL             -202
#define RT_ERR_APDU_STORE_DATA_FAIL             -203
#define RT_ERR_APDU_SEND_FAIL                   -204

#define RT_ERR_ASN1_ENCODE_FAIL                 -400
#define RT_ERR_ASN1_DECODE_FAIL                 -401

#define RT_ERR_CJSON_ERROR                      -500

#define RT_ERR_OUT_OF_MEMORY                    -600

#define RT_ERR_WRONG_AC_FORMAT                  -700
#define RT_ERR_NEED_CONFIRMATION_CODE           -701
#define RT_ERR_INITIATE_AUTHENTICATION          -702
#define RT_ERR_AUTHENTICATE_CLIENT              -703
#define RT_ERR_GET_BOUND_PROFILE_PACKAGE        -704
#define RT_ERR_HANDLE_NOTIFICATION              -705

#endif  // __LPA_ERROR_CODES_H__
