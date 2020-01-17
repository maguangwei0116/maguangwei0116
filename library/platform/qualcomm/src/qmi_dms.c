
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : qmi_dms.c
 * Date        : 2018.09.18
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <string.h>

#include "qmi_dms.h"
#include "qmi_control_point.h"

#define RT_DMS_V01_IDL_MINOR_VERS  0x39

static qmi_client_error_type dms_get_mfr_req(qmi_client_type dms_client, qmi_device_info_t *devinfo)
{
    qmi_client_error_type err;
    dms_get_device_mfr_req_msg_v01 req = { 0 };
    dms_get_device_mfr_resp_msg_v01 resp = { 0 };

    QMI_CLIENT_SEND_SYNC(err, dms_client, QMI_DMS_GET_DEVICE_MFR_REQ_V01, req, resp);

    if(err == QMI_NO_ERR) {
        rt_os_strcpy(devinfo->device_manufacturer, resp.device_manufacturer);
    }
    return err;
}

static qmi_client_error_type dms_get_device_model_id(qmi_client_type dms_client, qmi_device_info_t *devinfo)
{
    qmi_client_error_type err;
    dms_get_device_model_id_req_msg_v01 req = { 0 };
    dms_get_device_model_id_resp_msg_v01 resp = { 0 };

    QMI_CLIENT_SEND_SYNC(err, dms_client, QMI_DMS_GET_DEVICE_MODEL_ID_REQ_V01, req, resp);

    if (err == QMI_NO_ERR) {
        rt_os_strcpy(devinfo->device_model_id, resp.device_model_id);
    }
    return err;
}

static qmi_client_error_type dms_get_device_rev_id(qmi_client_type dms_client, qmi_device_info_t *devinfo)
{
    qmi_client_error_type err;
    dms_get_device_rev_id_req_msg_v01 req = { 0 };
    dms_get_device_rev_id_resp_msg_v01 resp = { 0 };

    QMI_CLIENT_SEND_SYNC(err, dms_client, QMI_DMS_GET_DEVICE_REV_ID_REQ_V01, req, resp);

    if (err == QMI_NO_ERR) {
        rt_os_strcpy(devinfo->device_rev_id, resp.device_rev_id);
        devinfo->boot_code_rev_valid = resp.boot_code_rev_valid;
        if (resp.boot_code_rev_valid) {
            rt_os_strcpy(devinfo->boot_code_rev, resp.boot_code_rev);
        }
    }
    return err;
}

qmi_client_error_type dms_get_device_serial_numbers(qmi_client_type dms_client, qmi_device_info_t *devinfo)
{
    qmi_client_error_type err;
    dms_get_device_serial_numbers_req_msg_v01 req = { 0 };
    dms_get_device_serial_numbers_resp_msg_v01 resp = { 0 };

    QMI_CLIENT_SEND_SYNC(err, dms_client, QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01, req, resp);

    if (err == QMI_NO_ERR) {
        devinfo->imei_valid = resp.imei_valid;
        if (resp.imei_valid) {
            rt_os_strcpy(devinfo->imei, resp.imei);
        }
    }
    return err;
}

static qmi_client_error_type dms_get_msisdn_req(qmi_client_type dms_client, qmi_device_info_t *devinfo)
{
    qmi_client_error_type err;
    dms_get_msisdn_req_msg_v01 req = { 0 };
    dms_get_msisdn_resp_msg_v01 resp = { 0 };

    QMI_CLIENT_SEND_SYNC(err, dms_client, QMI_DMS_GET_MSISDN_REQ_V01, req, resp);

    if (err == QMI_NO_ERR) {
        rt_os_strcpy(devinfo->voice_number, resp.voice_number);
        devinfo->mobile_id_number_valid = resp.mobile_id_number_valid;
        if(resp.mobile_id_number_valid) {
            rt_os_strcpy(devinfo->mobile_id_number, resp.mobile_id_number);
        }
        devinfo->imsi_valid = resp.imsi_valid;
        if(resp.imsi_valid) {
            rt_os_strcpy(devinfo->imsi, resp.imsi);
        }
    }
    return err;
}

void dump_device_info(qmi_device_info_t *devinfo)
{
    MSG_PRINTF(LOG_WARN, "Manufacture  : %s\n", devinfo->device_manufacturer);
    MSG_PRINTF(LOG_WARN, "Modem ID     : %s\n", devinfo->device_model_id);
    MSG_PRINTF(LOG_WARN, "Revision ID  : %s\n", devinfo->boot_code_rev_valid ? devinfo->boot_code_rev : "N/A");
    MSG_PRINTF(LOG_WARN, "IMEI         : %s\n", devinfo->imei_valid ? devinfo->imei : "N/A");
    MSG_PRINTF(LOG_WARN, "Voice Number : %s\n", devinfo->mobile_id_number_valid ? devinfo->mobile_id_number : "N/A");
    MSG_PRINTF(LOG_WARN, "IMSI         : %s\n", devinfo->imsi_valid ? devinfo->imsi : "N/A");
}

int qmi_query_device_info(qmi_device_info_t *devinfo)
{
    qmi_client_error_type err;
    qmi_client_type dms_client;
    qmi_idl_service_object_type idl_service_object;

    idl_service_object = dms_get_service_object_internal_v01(DMS_V01_IDL_MAJOR_VERS,RT_DMS_V01_IDL_MINOR_VERS,DMS_V01_IDL_TOOL_VERS);
    err = qmi_ctrl_point_init(idl_service_object, &dms_client, NULL, NULL);
    if (err != QMI_NO_ERR) {
        MSG_PRINTF(LOG_WARN, "failed to initialize control point of DMS: %d\n", err);
        return err;
    }
    
    if( (err = dms_get_mfr_req(dms_client, devinfo)) != QMI_NO_ERR) goto out;
    if( (err = dms_get_device_model_id(dms_client, devinfo)) != QMI_NO_ERR) goto out;
    if( (err = dms_get_device_rev_id(dms_client, devinfo)) != QMI_NO_ERR) goto out;
    if( (err = dms_get_device_serial_numbers(dms_client, devinfo)) != QMI_NO_ERR) goto out;
    if( (err = dms_get_msisdn_req(dms_client, devinfo)) != QMI_NO_ERR) goto out;
out:
    qmi_client_release(dms_client);
    return err;
}
