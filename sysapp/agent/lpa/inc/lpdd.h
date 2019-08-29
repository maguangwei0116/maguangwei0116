#ifndef __LPDD_H__
#define __LPDD_H__

#include <stdbool.h>
#include "rt_type.h"

#define API_INITIATE_AUTHENTICATION             "/gsma/rsp2/es9plus/initiateAuthentication"
#define API_AUTHENTICATE_CLIENT                 "/gsma/rsp2/es9plus/authenticateClient"
#define API_GET_BOUND_PROFILE_PACKAGE           "/gsma/rsp2/es9plus/getBoundProfilePackage"
#define API_HANDLE_NOTIFICATION                 "/gsma/rsp2/es9plus/handleNotification"
#define API_CANCEL_SESSION                      "/gsma/rsp2/es9plus/cancelSession"

typedef enum notification_event {
    NE_INSTALL,
    NE_ENABLE,
    NE_DISABLE,
    NE_DELETE,
    NE_SEQ_NUM,
    NE_ALL
} notification_t;

// ES10b
int get_rat(uint8_t *rat, uint16_t *size, int8_t channel);
int get_euicc_challenge(uint8_t challenge[16], int8_t channel);
int get_euicc_info(uint8_t *info1, uint16_t *size1, uint8_t *info2, uint16_t *size2, int8_t channel);
int cancel_session(const uint8_t *tid, uint8_t tid_size, uint8_t reason, uint8_t *csr, uint16_t *size, int8_t channel);
int list_notification(notification_t ne, uint8_t *out, uint16_t *out_size, int8_t channel);
int load_crl(const uint8_t *crl, uint16_t crl_size, uint8_t *out, uint16_t *out_size, int8_t channel);
int remove_notification_from_list(long seq, uint8_t *out, uint16_t *out_size, int8_t channel);
int retrieve_notification_list(notification_t ne, long *seq, uint8_t *out, uint16_t *out_size, int8_t channel);
int authenticate_server(const char *matching_id, const char *auth_data,
                        uint8_t *response, uint16_t *size , int8_t channel/* out */);

int prepare_download(const char *req_str, const char *cc, uint8_t *out, uint16_t *out_size, int8_t channel);
int load_bound_profile_package(const char *smdp_addr, const char *get_bpp_rsp,
                                uint8_t *out, uint16_t *out_size , int8_t channel/* out */);

// ES9+
int initiate_authentication(const char *smdp_addr, char *auth_data, int *size, int8_t channel);
int authenticate_client(const char *smdp_addr, const uint8_t *in, uint16_t in_size,
                        char *out, int *out_size/* out */);
int get_bound_profile_package(const char *smdp_addr, const uint8_t *in, uint16_t in_size,
                        char *out, int *out_size /* out */);
int handle_notification(const char *smdp_addr, const uint8_t *in, uint16_t in_size,
                        char *out, int *out_size /* out */);
int es9p_cancel_session(const char *smdp_addr, const uint8_t *in, uint16_t in_size,
                        char *out, int *out_size /* out */);
void close_session(void);

// Additional
int load_cert(const uint8_t *data, uint16_t data_len, int8_t channel);
int load_profile(const uint8_t *data, uint16_t data_len, int8_t channel);
#endif  // __LPDD_H__
