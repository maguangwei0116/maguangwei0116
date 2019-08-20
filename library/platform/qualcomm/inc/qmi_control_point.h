/*
 * qmi_control_point.h
 *
 *  Created on: Aug 29, 2018
 *      Author: ryan
 */

#ifndef QMI_CONTROL_POINT_H_
#define QMI_CONTROL_POINT_H_

#include <qmi-framework/qmi_client.h>
#include "rt_qmi.h"

qmi_client_error_type qmi_ctrl_point_init(
        qmi_idl_service_object_type idl_service_object,
        qmi_client_type *qmi_client,
        qmi_client_ind_cb client_ind_cb, void *client_ind_cb_data
);

#define QMI_CLIENT_DEFAULT_SYNC_MSG_TIMEOUT     (5000)

#define QMI_CLIENT_SEND_SYNC_TMO(_err, _client, _id, _req, _resp, _tmo) \
    do { \
        _err = qmi_client_send_msg_sync(_client, _id, \
                (void *)&_req, sizeof(_req), \
                (void *)&_resp, sizeof(_resp), \
                _tmo); \
        if(_err != QMI_NO_ERR) MSG_PRINTF(LOG_ERR, "qmi_client_send_msg_sync failed on request %s: %d\n", #_id, _err); \
    } while(0)

#define QMI_CLIENT_SEND_SYNC(_err, _client, _id, _req, _resp) \
    QMI_CLIENT_SEND_SYNC_TMO(_err, _client, _id, _req, _resp, QMI_CLIENT_DEFAULT_SYNC_MSG_TIMEOUT)

#endif // QMI_CONTROL_POINT_H_
