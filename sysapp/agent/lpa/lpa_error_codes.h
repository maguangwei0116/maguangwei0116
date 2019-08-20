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

#define RT_ERR_HTTPS_GET_HOSTNAME_FAIL          -300
#define RT_ERR_HTTPS_GET_SOCKET_FAIL            -301
#define RT_ERR_HTTPS_CONNECT_SOCKET_FAIL        -302
#define RT_ERR_HTTPS_NEW_SSL_CTX_FAIL           -303
#define RT_ERR_HTTPS_CLIENT_CRT_NOT_FOUND       -304
#define RT_ERR_HTTPS_SERVER_CRT_NOT_FOUND       -305
#define RT_ERR_HTTPS_SERVER_CRT_VERIFY_FAIL     -306
#define RT_ERR_HTTPS_CREATE_SSL_FAIL            -307
#define RT_ERR_HTTPS_CONNECT_SSL_FAIL           -308
#define RT_ERR_HTTPS_SSL_HANDSHAKE_FAIL         -309
#define RT_ERR_HTTPS_POST_FAIL                  -310
#define RT_ERR_HTTPS_SMDP_ERROR                 -311

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

// Base64
#define RT_ERR_BASE64_BAD_MSG                   -2002

// Converter
#define RT_ERR_CONVER_BAD_LEN                   -3002
#define RT_ERR_CONVER_BAD_CHAR                  -3003

#endif  // __LPA_ERROR_CODES_H__
