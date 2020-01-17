
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : qmi_uim.c
 * Date        : 2018.09.18
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "rt_os.h"
#include "qmi_uim.h"
#include "qmi_control_point.h"

static qmi_client_type g_uim_client;

int qmi_uim_init(void)
{
    qmi_client_error_type err;
    
    err = qmi_ctrl_point_init(uim_get_service_object_v01(), &g_uim_client, NULL, NULL);
    if(err != QMI_NO_ERR) {
        MSG_PRINTF(LOG_WARN, "failed to initialize control point of UIM: %d\n", err);
    }
    return err;
}

int qmi_get_elementary_iccid_file(uint8_t *iccid)
{
    qmi_client_error_type err;
    uim_read_record_req_msg_v01 req = { 0 };
    uim_read_record_resp_msg_v01 resp = { 0 };

    req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;

    req.file_id.file_id = 0x2fe2;
    req.file_id.path_len = 2;
    req.file_id.path[0] = 0x00;
    req.file_id.path[1] = 0x3f;

    req.read_record.record = 1;
    req.read_record.length = 0;

    QMI_CLIENT_SEND_SYNC(err, g_uim_client, QMI_UIM_READ_RECORD_REQ_V01, req, resp);
    if(err == QMI_NO_ERR) {
        if(resp.resp.result != QMI_RESULT_SUCCESS_V01) {
            MSG_PRINTF(LOG_WARN, "Failed to get ICCID from Elementary File 0x%04x, result: %d, error code: %d\n",
                    req.file_id.file_id,
                    resp.resp.result, resp.resp.error);
            err = resp.resp.result;
            goto out;
        }
        if(resp.read_result_valid) {
            get_ascii_data(resp.read_result.content, resp.read_result.content_len, iccid);
        }
    }
out:
    return err;
}

int qmi_get_elementary_imsi_file(uint8_t *imsi)
{
    qmi_client_error_type err;
    uim_read_record_req_msg_v01 req = { 0 };
    uim_read_record_resp_msg_v01 resp = { 0 };

    req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;

    req.file_id.file_id = 0x6f07;
    req.file_id.path_len = 4;
    req.file_id.path[0] = 0x00;
    req.file_id.path[1] = 0x3f;
    req.file_id.path[2] = 0xff;
    req.file_id.path[3] = 0x7f;

    req.read_record.record = 1;
    req.read_record.length = 0;

    rt_os_memset(&resp, 0, sizeof(resp));

    QMI_CLIENT_SEND_SYNC(err, g_uim_client, QMI_UIM_READ_RECORD_REQ_V01, req, resp);
    if (err == QMI_NO_ERR) {
        if(resp.resp.result != QMI_RESULT_SUCCESS_V01) {
            MSG_PRINTF(LOG_WARN, "Failed to get IMSI from Elementary File 0x%04x, result: %d, error code: %d\n",
                    req.file_id.file_id,
                    resp.resp.result, resp.resp.error);
            err = resp.resp.result;
            goto out;
        }
        if(resp.read_result_valid) {
            int len;
            if(resp.read_result.content_len > 0) {
                len = resp.read_result.content[0];
                get_ascii_data(&resp.read_result.content[1], len, imsi);
                rt_os_strcpy(imsi, imsi + 1);
            }
        }
    }
out:
    return err;
}

int qmi_open_channel(const uint8_t *aid, uint16_t aid_len, uint8_t *channel)
{
    qmi_client_error_type err;
    uim_logical_channel_req_msg_v01 req = { 0 };
    uim_logical_channel_resp_msg_v01 resp = { 0 };

    rt_os_memset(&resp, 0, sizeof(resp));
    rt_os_memset(&req, 0, sizeof(req));

    req.slot = 1;
    req.aid_valid = 1;
    req.aid_len = aid_len;
    rt_os_memcpy(req.aid, aid, req.aid_len);
    QMI_CLIENT_SEND_SYNC(err, g_uim_client, QMI_UIM_LOGICAL_CHANNEL_REQ_V01, req, resp);
    if(err == QMI_NO_ERR) {
        if(resp.resp.result != QMI_RESULT_SUCCESS_V01) {
            MSG_PRINTF(LOG_ERR, "Failed to open channel\n");
            err = resp.resp.result;
        }
        if(resp.channel_id_valid) {     // analyse respose
            *channel = resp.channel_id;
        }
    }
    return err;
}

int qmi_close_channel(uint8_t channel)
{
    qmi_client_error_type err;
    uim_logical_channel_req_msg_v01 req = { 0 };
    uim_logical_channel_resp_msg_v01 resp = { 0 };
    
    req.slot = 1;
    req.channel_id_valid = 1;
    req.channel_id = channel;

    QMI_CLIENT_SEND_SYNC(err, g_uim_client, QMI_UIM_LOGICAL_CHANNEL_REQ_V01, req, resp);
    if(err == QMI_NO_ERR) {
        if(resp.resp.result != QMI_RESULT_SUCCESS_V01) {
             MSG_PRINTF(LOG_ERR, "Failed to close channel:0x%x\n",resp.resp.error);
            if (resp.resp.error == QMI_ERR_INVALID_ARG_V01) {      //euicc already close the channel.
                resp.resp.error = QMI_NO_ERR;
            }
            err = resp.resp.error;
        }
        if(resp.channel_id_valid) {     // analyse respose

        }
    }
    return err;
}

int qmi_send_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel)
{
    qmi_client_error_type err;
    uim_send_apdu_req_msg_v01 req = { 0 };
    uim_send_apdu_resp_msg_v01 resp = { 0 };

    req.slot=1;
    req.apdu_len = data_len;
    rt_os_memcpy(req.apdu, data, data_len);
    req.apdu[req.apdu_len] = '\0';
    req.channel_id_valid = 1;
    req.channel_id = channel;
    MSG_INFO_ARRAY("APDU REQ: ",req.apdu,req.apdu_len);
    QMI_CLIENT_SEND_SYNC(err, g_uim_client, QMI_UIM_SEND_APDU_REQ_V01, req, resp);
    if(err == QMI_NO_ERR) {
        if(resp.resp.result != QMI_RESULT_SUCCESS_V01) {
            MSG_PRINTF(LOG_ERR, "Failed to send apdu\n");
            err = resp.resp.error;
        }
        if(resp.apdu_valid) {        //analyse respose
            *rsp_len = resp.apdu_len;
            rt_os_memcpy(rsp,resp.apdu,*rsp_len);
            MSG_INFO_ARRAY("APDU RSP: ",resp.apdu,resp.apdu_len);
        }
    }
    return err;
}