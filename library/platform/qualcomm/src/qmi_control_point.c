
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : qmi_control_point.c
 * Date        : 2018.09.18
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include "qmi_control_point.h"

qmi_client_error_type qmi_ctrl_point_init(
        qmi_idl_service_object_type idl_service_object,
        qmi_client_type *qmi_client,
        qmi_client_ind_cb client_ind_cb, void *client_ind_cb_data)
{
    qmi_client_error_type err;
    qmi_service_info service_info;
    uint32_t num_services = 0, num_entries = 0;
    static error_time = 0;

    if (idl_service_object == NULL) {
        MSG_PRINTF(LOG_WARN, "Invalid IDL_SERVICE_OBJECT\n");
        return -1;
    }

    err = qmi_client_get_service_list(idl_service_object, NULL, NULL, &num_services);
    if (err != QMI_NO_ERR) {
        MSG_PRINTF(LOG_WARN, "qmi_client_get_service_list failed: %d\n", err);
        return err;
    }
    //Populate service info
    num_entries = num_services;
    err = qmi_client_get_service_list(idl_service_object, &service_info, &num_entries, &num_services);
    if (err != QMI_NO_ERR) {
        MSG_PRINTF(LOG_WARN, "qmi_client_get_service_list failed: %d\n", err);
        return err;
    }
    //Initialize qmi_client
    err = qmi_client_init(&service_info, idl_service_object,
            client_ind_cb, client_ind_cb_data, NULL, qmi_client);

    if (err != QMI_NO_ERR) {
        error_time ++;
        if (error_time>=10) {
            rt_os_reboot();
        }
        MSG_PRINTF(LOG_WARN, "qmi_client_init result %d\n", err);
    } else {
        error_time = 0;
    }
    return err;
}

