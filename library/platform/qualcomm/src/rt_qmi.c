
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_qmi.c
 * Date        : 2018.09.18
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include "rt_type.h"
#include "rt_qmi.h"
#include "qmi_uim.h"
#include "qmi_dms.h"
#include "qmi_nas.h"
#include "qmi_wds.h"

int32_t rt_qmi_send_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel)
{
    return qmi_send_apdu(data, data_len, rsp, rsp_len, channel);
}

int32_t rt_qmi_close_channel(uint8_t channel)
{
    return qmi_close_channel(channel);  
}

int32_t rt_qmi_open_channel(const uint8_t *aid, uint16_t aid_len, uint8_t *channel)
{
    return qmi_open_channel(aid, aid_len, channel);   
}

int32_t rt_qmi_get_register_state(int32_t *register_state)
{
    qmi_serving_system_info_t info;
    int32_t ret = RT_ERROR;

    if (register_state) {
        ret = qmi_get_serving_system(&info);
        if (ret == RT_SUCCESS) {
            *register_state = info.serving_system.registration_state;
        }
    }

    return ret;
}

int32_t rt_qmi_get_mcc_mnc(uint16_t *mcc, uint16_t *mnc)
{
    qmi_serving_system_info_t info;
    int32_t ret = RT_ERROR;

    ret = qmi_get_serving_system(&info);
    if (ret == RT_SUCCESS) {
        if (mcc) {
            *mcc = info.mnc_includes_pcs_digit.mcc;
        }
        if (mnc) {
            *mnc = info.mnc_includes_pcs_digit.mnc;
        }
    }

    return ret;
}

int32_t rt_qmi_get_current_iccid(char *iccid, int32_t size)
{
    int32_t ret = RT_ERROR;

    if (size < MIN_ICCID_LEN || !iccid) {
        return ret;
    }

    ret = qmi_get_elementary_iccid_file(iccid);

    return ret;
}

int32_t rt_qmi_get_current_imsi(char *imsi, int32_t size)
{
    int32_t ret = RT_ERROR;

    if (size < MIN_IMSI_LEN || !imsi) {
        return ret;
    }

    ret = qmi_get_elementary_imsi_file(imsi);

    return ret;
}

int32_t rt_qmi_get_signal(int32_t *strength)
{
    qmi_signal_strength_info_t info = {0};
    int32_t ret = RT_ERROR;

    if (strength) {   
        ret = qmi_get_signal_strength(&info);
        if (ret == RT_SUCCESS) {
            *strength = info.signal_strength;
        }
    }
    
    return ret;
}

int32_t rt_qmi_get_signal_level(int32_t *level)
{
    (void)level;
    return (!level) ? RT_ERROR : RT_SUCCESS;
}

int32_t rt_qmi_get_imei(char *imei, int32_t size)
{
    qmi_device_info_t devinfo;
    uint8_t ii = 0;
    int32_t ret = RT_ERROR;

    if (size < MIN_IMEI_LEN || !imei) {
        return ret;
    }
    
    while (ret != RT_SUCCESS) {
        ret = qmi_query_device_info(&devinfo);
        ii++;
        if (ii > 3) {
            break;
        }
    }
    if (ret == RT_SUCCESS) {
        rt_os_memcpy(imei, devinfo.imei, rt_os_strlen(devinfo.imei));
        imei[rt_os_strlen(devinfo.imei)] = '\0';
    }
    return ret;
}

int32_t rt_qmi_init(void *arg)
{
    qmi_nas_init();
    qmi_uim_init();

    return RT_SUCCESS;
}

// get MEID
int32_t rt_qmi_get_model(char *model, int32_t size)
{
    int32_t ret = RT_ERROR;
    qmi_device_info_t devinfo;

    if (size < MIN_MODEL_LEN) {
        return ret;
    }
    
    if (qmi_query_device_info(&devinfo) == RT_SUCCESS) {
        rt_os_memcpy(model, devinfo.device_model_id, rt_os_strlen(devinfo.device_model_id));
        ret = RT_SUCCESS;
    }
    return ret;
}

static const char *radio_str[] =
{
    "No Service",
    "cdma2000 1X",
    "cdma2000 HRPD (1xEV-DO)",
    "AMPS",
    "GSM",
    "UMTS",
    "",
    "",
    "LTE",
    "TD-SCDMA",
};

//get network type, return type is the index of radio_str array
static rt_bool qmi_get_radio_interface(int8_t *type)
{
    rt_bool ret = RT_FALSE;
    qmi_serving_system_info_t info;
    int32_t index = 0;
    int32_t err;

    err = qmi_get_serving_system(&info);
    if (err == RT_SUCCESS) {
        index = info.serving_system.radio_if[info.serving_system.radio_if_len - 1];
        MSG_PRINTF(LOG_INFO, "Radio Interface %d: %s\n", info.serving_system.radio_if_len + 1,
                info.serving_system.radio_if[index] <= 9 ?
                radio_str[index] : "Invalid value");
        if (type != NULL) {
            *type = index;
        }
        ret = RT_TRUE;
    }
    
    return ret;
}

// get network type
int32_t rt_qmi_get_network_type(char *network_type, int32_t size)
{
    int32_t ret = RT_ERROR;
    int8_t radio_index;
    int32_t type;

    if (size < MIN_NETWORK_TYPE_LEN) {
        return ret;
    }

    ret = qmi_get_radio_interface(&radio_index);
    if (RT_TRUE == ret) {
        switch(radio_index) {
            case 8 :
            case 9 :
                type = 7;
                break;

            case 4:
            case 5:
                type = 0;
                break;

            default:
                type = 5;
                break;
        }

        if(type == 7){
            rt_os_strcpy((char *)network_type, "4G");
        } else if ((type > 3) && (type < 7)) {
            rt_os_strcpy((char *)network_type, "3G");
        } else {
            rt_os_strcpy((char *)network_type, "2G");
        }
        ret = RT_SUCCESS;
    } else {
        ret = RT_ERROR;
    }

    return ret;
}

/*****************************************************************************
 * FUNCTION
 *  rt_modify_profile
 * DESCRIPTION
 *  used to modify wireless service data profile
 * PARAMETERS
*   index : profile index
*   profile_type
*       0: 3GPP
*       1: 3GPP2
*       2: EPC
*   pdp_type
*       0: IPv4
*       1: IPv6
*       2: IPv4v6
*   apn : APN used to attach LTE
*   mccmnc: special for android set apn API
 * RETURNS
 *  int
 *****************************************************************************/
int32_t rt_qmi_modify_profile(int8_t index, int8_t profile_type, int8_t pdp_type, const char *apn, const char *mcc_mnc)
{
    qmi_wds_profile_info_t info;
    uint8_t ii = 0;
    int32_t ret = RT_ERROR;

    (void)mcc_mnc;
    info.apn_name       = (char *)apn;
    info.pdp_type       = pdp_type;
    info.profile_index  = index;
    info.profile_type   = profile_type;

    while (ret != RT_SUCCESS) {
        ret = qmi_modify_profile(&info);
        ii++;
        if (ii > 3) {
            break;
        }
    }

    MSG_PRINTF(LOG_INFO, "set apn: %s [%s] ret: %d\r\n", (char *)apn, (char *)mcc_mnc, ret);
    
    return ret;
}
