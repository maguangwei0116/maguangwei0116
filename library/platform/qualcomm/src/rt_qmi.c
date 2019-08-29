
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

#include "rt_qmi.h"
#include "qmi_uim.h"
#include "qmi_dms.h"
#include "qmi_nas.h"
#include "qmi_wds.h"

int32_t rt_qmi_get_register_state(int32_t *register_state)
{
    qmi_serving_system_info_t info;
    int32_t ret = 0;
    ret = qmi_get_serving_system(&info);
    if (ret == 0) {
        *register_state = info.serving_system.registration_state;
    }
    return ret;
}

int32_t rt_qmi_get_mcc_mnc(uint16_t *mcc, uint16_t *mnc)
{
    qmi_serving_system_info_t info;
    int32_t ret = 0;
    ret = qmi_get_serving_system(&info);
    if (ret == 0) {
        if (mcc) {
            *mcc = info.mnc_includes_pcs_digit.mcc;
        }
        if (mnc) {
            *mnc = info.mnc_includes_pcs_digit.mnc;
        }
    }
    return ret;
}

int32_t rt_qmi_get_current_iccid(uint8_t *iccid)
{
    int32_t ret = 0;
    if (iccid != NULL) {
        ret = qmi_get_elementary_iccid_file(iccid);
    }
    return ret;
}

int32_t rt_qmi_get_current_imsi(uint8_t *imsi)
{
    int32_t ret = 0;
    if (imsi != NULL) {
        ret = qmi_get_elementary_imsi_file(imsi);
    }
    return ret;
}

int32_t rt_qmi_get_signal(int32_t *strength)
{
    qmi_signal_strength_info_t info;
    int32_t ret = 0;
    ret = qmi_get_signal_strength(&info);
    if (ret == 0) {
        *strength = info.signal_strength;
    }
    return ret;
}

int32_t rt_qmi_get_imei(uint8_t *imei)
{
    qmi_device_info_t devinfo;
    uint8_t ii = 0;
    int32_t ret = RT_ERROR;
    while (ret != 0) {
        ret = qmi_query_device_info(&devinfo);
        ii++;
        if (ii > 3) {
            break;
        }
    }
    if (ret == 0) {
        rt_os_memcpy(imei, devinfo.imei, rt_os_strlen(devinfo.imei));
        imei[rt_os_strlen(devinfo.imei)] = '\0';
    }
    return ret;
}

int32_t rt_qmi_init(void *arg)
{
    qmi_nas_init();
    qmi_uim_init();
}

/*****************************************************************************
 * FUNCTION
 *  rt_modify_profile
 * DESCRIPTION
 *  used to modify wireless service data profile
 * PARAMETERS
*   index : profile index
*   profile_type
*       0：3GPP
*       1：3GPP2
*       2：EPC
*   apn : APN used to attach LTE
*   pdp_type
*       0：IPv4
*       1：IPv6
*       2: IPv4v6
 * RETURNS
 *  int
 *****************************************************************************/
int32_t rt_qmi_modify_profile(int8_t index, int8_t profile_type, int8_t *apn, int8_t pdp_type)
{
    qmi_wds_profile_info_t info;
    uint8_t ii = 0;
    int32_t ret = RT_ERROR;
    info.apn_name = apn;
    info.pdp_type = pdp_type;
    info.profile_index = index;
    info.profile_type = profile_type;
    while (ret != 0) {
        ret = qmi_modify_profile(&info);
        ii++;
        if (ii > 3) {
            break;
        }
    }
    return ret;
}
