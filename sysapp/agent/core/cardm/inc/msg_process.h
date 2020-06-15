
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : msg_process.h
 * Date        : 2019.09.04
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __MSG_PROCESS_H__
#define __MSG_PROCESS_H__

#include "rt_type.h"
#include "cJSON.h"

#define SUCCESS_STATUS                               0000
#define ERROR_PARSE_STATUS                           1001
#define ERROR_INSERT_COS_STATUS                      1002
#define ERROR_ENABLE_CARD_STATUS                     1003
#define ERROR_REGIST_STATUS                          1004
#define ERROR_REGISTED_NONET_STATUS                  1005
#define ERROR_NO_ICCID_STATUS                        1006
#define ERROR_DISABLE_ONLY_ONE_ICCID_STATUS          1007
#define ERROR_DELETE_ONLY_ONE_ICCID_STATUS           1008
#define ERROR_CAN_NOT_DELETE_CARD_STATUS             1009
#define ERROR_CAN_NOT_FIND_CARD_STATUS               1010
#define ERROR_DISABLE_DATA_STATUS                    1011
#define ERROR_DELETE_USED_CARD                       1012
#define ERROR_DELETE_DATA_STATUS                     1013
#define ERROR_CREATE_CARD_STATUS                     1014
#define ERROR_SLOT_NOT_SUPPORT_STATUS                1015
#define ERROR_NO_APN_LIST                            1020
#define ERROR_NO_REQUIRED_DATA                       1021
#define ERROR_CARD_OVERFLOW                          1023
#define ERROR_MALLOC                                 1024
#define ERROR_VERSION_INFO                           2001
#define ERROR_DOWNLOAD_FILE                          2002
#define ERROR_CHECK_HASH                             2003
#define ERROR_REPLACE_FILE                           2004
#define ERROR_FILE_NAME                              2005
#define ERROR_CONFIG                                 1030

typedef enum JUDGE_TERM {
    UPDATE_NOT_JUDGE_BOOTSTRAP = 0,
    UPDATE_JUDGE_BOOTSTRAP
} judge_term_e;

int32_t mqtt_msg_event(const uint8_t *buf, int32_t len);
int32_t msg_download_profile(const char *ac, const char *cc, char iccid[21], int32_t avariable_num);
int32_t msg_enable_profile(const char *iccid);
int32_t msg_delete_profile(const char *iccid, rt_bool *opr_iccid_using);
int32_t card_update_profile_info(judge_term_e bootstrap_flag);
int32_t init_msg_process(void *arg, void *proxy_addr);
int32_t msg_set_apn(const char *iccid);
int32_t msg_analyse_apn(cJSON *command_content, uint8_t *iccid);
int32_t msg_apnlist_handler(cJSON *apnparams_list);
int32_t config_update_device_key(const char *devicekey);
int32_t msg_analyse_strategy(cJSON *command_content);
void msg_monitorstrategy_handler(cJSON *monitorstrategyparams);
rt_bool inspect_device_key(const char *file_name);

#endif // __MSG_PROCESS_H__
