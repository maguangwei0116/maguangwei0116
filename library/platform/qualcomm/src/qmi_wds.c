
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : qmi_wds.c
 * Date        : 2018.11.04
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <stdlib.h>
#include "qmi_wds.h"
#include "qmi_control_point.h"

int qmi_modify_profile(qmi_wds_profile_info_t *info)
{
    qmi_client_error_type err;
    qmi_client_type wds_client;
    wds_modify_profile_settings_req_msg_v01 req = { 0 };
    wds_modify_profile_settings_resp_msg_v01 resp = { 0 };
    err = qmi_ctrl_point_init(wds_get_service_object_v01(), &wds_client, NULL, NULL);
    if(err != QMI_NO_ERR) {
        MSG_PRINTF(LOG_WARN, "failed to initialize control point of WDS: %d", err);
       return err;
    }

    //choose apn link
    req.profile.profile_index = info->profile_index;
    req.profile.profile_type = info->profile_type;

    //set APN
    req.apn_name_valid = 1;
    rt_os_memcpy(req.apn_name, info->apn_name, rt_os_strlen(info->apn_name));
    req.apn_name[rt_os_strlen(info->apn_name)] = '\0';

    //set PDP type
    req.pdp_type_valid = 1;
    req.pdp_type = info->pdp_type;

    //set userame pwd
    //req.username_valid = false;
    //req.password_valid = false;
    //rt_os_memcpy(req.username, info.usrname, sizeof(info.usrname));
    //rt_os_memcpy(req.password, info.passwd, sizeof(info.passwd));

    QMI_CLIENT_SEND_SYNC(err, wds_client, QMI_WDS_MODIFY_PROFILE_SETTINGS_REQ_V01, req, resp);

    if(err == QMI_NO_ERR) {
        if(resp.resp.result != QMI_RESULT_SUCCESS_V01) {
            MSG_PRINTF(LOG_WARN, "Failed to modify %d profile, result: %d, error code: %d\n",
                    info->profile_index,
                    resp.resp.result, resp.resp.error);
            if(resp.extended_error_code_valid == 1) {
                MSG_PRINTF(LOG_WARN, "extended_error_code : %d\n", resp.extended_error_code);
            }
            err = resp.resp.result;
            goto out;
        }
    }
out:
    qmi_client_release(wds_client);
    return err;
}
