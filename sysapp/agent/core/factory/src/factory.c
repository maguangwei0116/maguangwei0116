/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : factory.c
 * Date        : 2021.03.03
 * Note        : 
 * Description :
 * Contributors:
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "factory.h"
#include "stdint.h"
#include "rt_type.h"
#include "convert.h"
#include "agent_queue.h"
#include "card_manager.h"

#ifdef CFG_FACTORY_MODE_ON

#define FACTORY_PROFILE                             "/oemapp/rt_factory_profile.der"
#define INVALID_PROFILE_INDEX                       (uint32_t)0xFFFFFFFF
#define MAX_PROFILE_INDEX                           100
#define CHECK_CODE_LENGT                            6
#define INDEX_LENGTH                                3
#define ACTIVE_CODE_LENGTH                          (CHECK_CODE_LENGT + INDEX_LENGTH)
#define ICCID_LENGTH                                10
#define ICCID_STR_LENGTH                            (ICCID_LENGTH << 1)

static const char  g_enc_code[CHECK_CODE_LENGT] = {0x21, 0x13, 0x1C, 0x1F, 0x20, 0x14};
static uint8_t     g_factory_mode;
static uint32_t    g_factory_profile_index;

static int32_t init_factory_profile(void)
{
    return init_profile_file(FACTORY_PROFILE);
}

int32_t init_factory(void *arg)
{
    //(public_value_list_t *)arg;

    g_factory_mode          = FACTORY_DISABLE;       /* factory mode init disable */
    g_factory_profile_index = INVALID_PROFILE_INDEX; /* factory profile index init invalid */

    return RT_SUCCESS;
}

static int32_t fetch_profile_index(const char* active_code, uint32_t *profile_index)
{
    char     *p = NULL;
    uint32_t index = 0;
    int32_t  ret = RT_ERROR;
    /* fetch profile index */
    if (rt_os_strlen(active_code) != ACTIVE_CODE_LENGTH) {
        MSG_PRINTF(LOG_ERR, "active code length is wrong.\n");
        goto end;
    }
    index = strtol(active_code + CHECK_CODE_LENGT, &p, 10);
    if (index == 0 || index > MAX_PROFILE_INDEX) {
        MSG_PRINTF(LOG_ERR, "profile index is out of range.\n");
        goto end;
    }
    *profile_index = index - 1;
    ret = RT_SUCCESS;
end:
    return ret;    
}

static int32_t is_check_code_valid(const char* check_code, uint8_t* iccid)
{
    char    str_chk_code[CHECK_CODE_LENGT+1];
    char    str_iccid[ICCID_STR_LENGTH+1];
    uint8_t ii;
    int32_t ret = RT_ERROR;

    rt_os_memset(str_iccid, 0, sizeof(str_iccid));
    if (bytes2hexstring(iccid, ICCID_LENGTH, str_iccid) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Convert iccid failed.\n");
        goto end;
    }
    rt_os_memset(str_chk_code, 0, sizeof(str_chk_code));
    for (ii = 0; ii < CHECK_CODE_LENGT; ii++) {
        str_chk_code[ii] = str_iccid[ICCID_STR_LENGTH - CHECK_CODE_LENGT + ii] + g_enc_code[ii];
    }
    str_chk_code[ii] = '\0';
    if (rt_os_strncmp(str_chk_code, check_code, CHECK_CODE_LENGT) != 0) {
        MSG_PRINTF(LOG_ERR, "Check code is invalid.\n");
        goto end;
    }    
    ret = RT_SUCCESS;
end:
    return ret;    
}

int32_t factory_mode_operation(const char* active_code)
{
    uint32_t profile_index = INVALID_PROFILE_INDEX;
    uint8_t  iccid[ICCID_STR_LENGTH+1];
    char     check_code[CHECK_CODE_LENGT+1];
    int32_t  ret = RT_ERROR;

    if (fetch_profile_index(active_code, &profile_index) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "fetch profile index failed, active_code:%s.\n", active_code);
        goto end;
    }
    if (init_factory_profile() != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "factory profile init failed.\n");
        goto end;
    }
    rt_os_memset(iccid, 0, sizeof(iccid));
    if (get_profile_iccid(profile_index, iccid) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "get profile iccid failed.\n");
        goto end;
    }
    rt_os_memset(check_code, 0, sizeof(check_code));
    rt_os_strncpy(check_code, active_code, CHECK_CODE_LENGT);
    check_code[CHECK_CODE_LENGT] = '\0';
    if (is_check_code_valid(check_code, iccid) != 0) {
        MSG_PRINTF(LOG_ERR, "active code is wrong.\n");
        goto end;
    }
    ret = card_force_enable_provisoning_profile();
    if ((ret != RT_SUCCESS) && (ret != 2)) {
        /* ret = 2 means profile not in disabled state */
        MSG_PRINTF(LOG_ERR, "switch to provisioning profile failed.\n");
        goto end;        
    }
    g_factory_profile_index = profile_index;
    msg_send_agent_queue(MSG_ID_BOOT_STRAP, MSG_BOOTSTRAP_SELECT_CARD, NULL, 0);
    g_factory_mode = FACTORY_ENABLE;
    MSG_PRINTF(LOG_TRACE, "factory mode enable.\n");
    ret = RT_SUCCESS;

end:
    return ret;
}

factory_mode_e factory_get_mode(void)
{
    return g_factory_mode;
}

uint32_t factory_get_profile_index(void)
{
    return g_factory_profile_index;
}

#endif // CFG_FACTORY_MODE_ON

