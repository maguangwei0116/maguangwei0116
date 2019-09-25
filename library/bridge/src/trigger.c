
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "trigger.h"
#include "qmi_idl_lib.h"
#include "qmi_client.h"
#include "qmi_cci_target_ext.h"

static qmi_client_type rm_uim_client = NULL;

typedef uint16_t (*trigger_callback_cmd)(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len);
typedef uint16_t (*trigger_callback_reset)(uint8_t *rsp, uint16_t *rsp_len);
trigger_callback_cmd trigger_cmd;
trigger_callback_reset trigger_reset;

void trigegr_regist_cmd(void *fun)
{
    trigger_cmd = (trigger_callback_cmd)fun;
}

void trigegr_regist_reset(void *fun)
{
    trigger_reset = (trigger_callback_reset)fun;
}

void remote_uim_async_cb(   qmi_client_type         user_handle,
                            unsigned int            msg_id,
                            void                    *resp_c_struct,
                            unsigned int            resp_c_struct_len,
                            void                    *resp_cb_data,
                            qmi_client_error_type   transp_err)
{
    if(QMI_UIM_REMOTE_GET_SUPPORTED_MSGS_RESP_V01 == msg_id){
        MSG_PRINTF(LOG_INFO,"QMI_UIM_REMOTE_GET_SUPPORTED_MSGS_RESP_V01\n");
    }
    else if(QMI_UIM_REMOTE_GET_SUPPORTED_FIELDS_RESP_V01 == msg_id){
        MSG_PRINTF(LOG_INFO,"QMI_UIM_REMOTE_GET_SUPPORTED_FIELDS_RESP_V01\n");
    }
    else if(QMI_UIM_REMOTE_RESET_RESP_V01 == msg_id){
        MSG_PRINTF(LOG_INFO,"QMI_UIM_REMOTE_RESET_RESP_V01 result:%d, error:%d\n",
                ((uim_remote_reset_resp_msg_v01 *)resp_c_struct)->resp.result,
                ((uim_remote_reset_resp_msg_v01 *)resp_c_struct)->resp.error);
    }
    else if(QMI_UIM_REMOTE_EVENT_RESP_V01 == msg_id){
        MSG_PRINTF(LOG_INFO,"QMI_UIM_REMOTE_EVENT_RESP_V01 result:%d, error:%d\n",
            ((uim_remote_event_resp_msg_v01 *)resp_c_struct)->resp.result,
            ((uim_remote_event_resp_msg_v01 *)resp_c_struct)->resp.error);
    }
    else if(QMI_UIM_REMOTE_APDU_RESP_V01 == msg_id){
        // Do not print info, we can do this in indication if we need
    }
}

static int process_ind_connect(qmi_client_type user_handle, unsigned int msg_id,
                        void *ind_buf, unsigned int ind_buf_len, qmi_txn_handle *txn)
{
    uim_remote_event_req_msg_v01 req = {0};
    uim_remote_event_resp_msg_v01 resp = {0};
    uim_remote_connect_ind_msg_v01 ind_msg = {0};
    int rc;

    MSG_PRINTF(LOG_INFO,"QMI_UIM_REMOTE_CONNECT_IND_V01\n");
    rc = qmi_client_message_decode(user_handle, QMI_IDL_INDICATION, msg_id, ind_buf,
                                    ind_buf_len, &ind_msg, sizeof(uim_remote_connect_ind_msg_v01));
    if(rc != RT_SUCCESS) {
        MSG_PRINTF(LOG_INFO,"qmi_client_message_decode failed rc = %d\n", rc);
        return rc;
    }

    req.event_info.event = UIM_REMOTE_CARD_RESET_V01;
    req.event_info.slot = ind_msg.slot;
    req.atr_valid = true;

    // fill ATR
    trigger_reset(req.atr, (uint16_t *)&req.atr_len);
    MSG_INFO_ARRAY("ATR:", req.atr, req.atr_len);

    rc = qmi_client_send_msg_async(user_handle, QMI_UIM_REMOTE_EVENT_REQ_V01, &req, sizeof(req),
                                    &resp, sizeof(resp), remote_uim_async_cb, NULL, txn);
    MSG_PRINTF(LOG_INFO,"UIM_REMOTE_CARD_RESET_V01 slot:%d, rc:%d\n", ind_msg.slot, rc);

    return rc;
}

static int process_ind_apdu(qmi_client_type user_handle, unsigned int msg_id,
                        void *ind_buf, unsigned int ind_buf_len, qmi_txn_handle *txn)
{
    uim_remote_apdu_req_msg_v01 req = {0};
    uim_remote_apdu_resp_msg_v01 resp = {0};
    uim_remote_apdu_ind_msg_v01 ind_msg = {0};
    int rc;
    uint16_t sw;
    rc = qmi_client_message_decode(user_handle, QMI_IDL_INDICATION, msg_id, ind_buf,
                        ind_buf_len, &ind_msg, sizeof(uim_remote_apdu_ind_msg_v01));
    if(rc != RT_SUCCESS) {
        MSG_PRINTF(LOG_INFO,"qmi_client_message_decode failed rc = %d", rc);
        return rc;
    }

    req.apdu_status = QMI_RESULT_SUCCESS_V01;
    req.slot = ind_msg.slot;
    req.apdu_id = ind_msg.apdu_id;

    MSG_INFO_ARRAY("M-APDU REQ:", ind_msg.command_apdu, ind_msg.command_apdu_len);

    // fill Response APDU
    req.response_apdu_segment_valid = true;

    sw = trigger_cmd(ind_msg.command_apdu , ind_msg.command_apdu_len,
                  req.response_apdu_segment, (uint16_t *)&req.response_apdu_segment_len);
    req.response_apdu_segment[req.response_apdu_segment_len++] = (sw >> 8) & 0xFF;
    req.response_apdu_segment[req.response_apdu_segment_len++] = sw & 0xFF;
    req.response_apdu_info_valid = true;
    req.response_apdu_info.total_response_apdu_size = req.response_apdu_segment_len;
    req.response_apdu_info.response_apdu_segment_offset = 0;

    MSG_INFO_ARRAY("M-APDU RSP:", req.response_apdu_segment, req.response_apdu_segment_len);

    rc = qmi_client_send_msg_async(user_handle, QMI_UIM_REMOTE_APDU_REQ_V01, &req, sizeof(req),
                                    &resp, sizeof(resp), remote_uim_async_cb, NULL, txn);

    return rc;
}

static int process_ind_pup(qmi_client_type user_handle, unsigned int msg_id,
                        void *ind_buf, unsigned int ind_buf_len, qmi_txn_handle *txn)
{
    uim_remote_event_req_msg_v01 req = {0};
    uim_remote_event_resp_msg_v01 resp = {0};
    uim_remote_card_power_up_ind_msg_v01 ind_msg = {0};
    int rc;

    MSG_PRINTF(LOG_INFO,"QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01\n");
    rc = qmi_client_message_decode(user_handle, QMI_IDL_INDICATION, msg_id, ind_buf,
                                    ind_buf_len, &ind_msg, sizeof(uim_remote_card_power_up_ind_msg_v01));
    if(rc != RT_SUCCESS) {
        MSG_PRINTF(LOG_INFO,"qmi_client_message_decode failed rc = %d\n", rc);
        return rc;
    }

    req.event_info.event = UIM_REMOTE_CARD_RESET_V01;
    req.event_info.slot = ind_msg.slot;
    req.atr_valid = true;

    // fill ATR
    trigger_reset(req.atr, (uint16_t *)&req.atr_len);
    //MSG_INFO_ARR2STR("ATR", req.atr, req.atr_len, 1);

    rc = qmi_client_send_msg_async(rm_uim_client, QMI_UIM_REMOTE_EVENT_REQ_V01, &req, sizeof(req),
                                    &resp, sizeof(resp), remote_uim_async_cb, NULL, txn);

    return rc;
}

static int process_ind_pdown(qmi_client_type user_handle, unsigned int msg_id,
                        void *ind_buf, unsigned int ind_buf_len, qmi_txn_handle *txn)
{
    // Do nothing
    return RT_SUCCESS;
}

static int process_ind_reset(qmi_client_type user_handle, unsigned int msg_id,
                        void *ind_buf, unsigned int ind_buf_len, qmi_txn_handle *txn)
{
    uim_remote_event_req_msg_v01 req = {0};
    uim_remote_event_resp_msg_v01 resp = {0};
    uim_remote_card_reset_ind_msg_v01 ind_msg = {0};
    int rc;

    MSG_PRINTF(LOG_INFO,"QMI_UIM_REMOTE_CARD_RESET_IND_V01\n");
    rc = qmi_client_message_decode(user_handle, QMI_IDL_INDICATION, msg_id, ind_buf,
                                    ind_buf_len, &ind_msg, sizeof(uim_remote_card_power_up_ind_msg_v01));
    if(rc != RT_SUCCESS) {
        MSG_PRINTF(LOG_INFO,"qmi_client_message_decode failed rc = %d\n", rc);
        return rc;
    }

    req.event_info.event = UIM_REMOTE_CARD_RESET_V01;
    req.event_info.slot = ind_msg.slot;
    req.atr_valid = true;

    // fill ATR
    trigger_reset(req.atr, (uint16_t *)&req.atr_len);
    MSG_INFO_ARRAY("ATR", req.atr, req.atr_len);

    rc = qmi_client_send_msg_async(user_handle, QMI_UIM_REMOTE_EVENT_REQ_V01, &req, sizeof(req),
                                    &resp, sizeof(resp), remote_uim_async_cb, NULL, txn);

    return rc;
}

void remote_uim_ind_cb( qmi_client_type       user_handle,
                        unsigned int          msg_id,
                        void                  *ind_buf,
                        unsigned int          ind_buf_len,
                        void                  *ind_cb_data)
{
    qmi_txn_handle txn;

    switch(msg_id) {
        case QMI_UIM_REMOTE_CONNECT_IND_V01:
            process_ind_connect(user_handle, msg_id, ind_buf, ind_buf_len, &txn);
            break;

        case QMI_UIM_REMOTE_DISCONNECT_IND_V01:
            MSG_PRINTF(LOG_INFO,"[QMI_UIM_REMOTE_DISCONNECT_IND_V01]\n");
            break;

        case QMI_UIM_REMOTE_APDU_IND_V01:
            process_ind_apdu(user_handle, msg_id, ind_buf, ind_buf_len, &txn);
            break;

        case QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01:
            process_ind_pup(user_handle, msg_id, ind_buf, ind_buf_len, &txn);
            break;

        case QMI_UIM_REMOTE_CARD_POWER_DOWN_IND_V01:
            process_ind_pdown(user_handle, msg_id, ind_buf, ind_buf_len, &txn);

        case QMI_UIM_REMOTE_CARD_RESET_IND_V01:
            process_ind_reset(user_handle, msg_id, ind_buf, ind_buf_len, &txn);
            break;
    }
}

int t9x07_insert_card(uim_remote_slot_type_enum_v01 slot)
{
    int rc;
    uim_remote_event_req_msg_v01 req = {0};
    uim_remote_event_resp_msg_v01 resp = {0};
    qmi_txn_handle txn;
    qmi_cci_os_signal_type os_params;
    qmi_idl_service_object_type remote_uim_service_object;

    unsigned int num_services, num_entries = 0;
    qmi_service_info info[10];

    if(slot != UIM_REMOTE_SLOT_1_V01) {
        MSG_PRINTF(LOG_ERR, "SLOT%d NOT SUPPORT CURRENTLY!\n", slot);
        return ERR_QMI_UNSUPPORTED_SLOT;
    }

    remote_uim_service_object = uim_remote_get_service_object_v01();
    MSG_PRINTF(LOG_INFO,"uim_remote_get_service_object_v01\n");
    if(remote_uim_service_object == NULL) {
        MSG_PRINTF(LOG_ERR, "no remote uim service object\n");
        return ERR_QMI_RUIM_SERVICE_OBJ;
    }

    while (1) {     // TODO: try to remove dead loop
        rc = qmi_client_get_service_list(remote_uim_service_object, NULL, NULL, &num_services);
        MSG_PRINTF(LOG_INFO,"rc: %d, num_services: %d\n", rc, num_services);
        if(QMI_NO_ERR == rc){
            break;
        }
        QMI_CCI_OS_SIGNAL_WAIT(&os_params, 0);
    }

    num_entries = num_services;

    rc = qmi_client_get_service_list(remote_uim_service_object, info, &num_entries, &num_services);
    MSG_PRINTF(LOG_INFO,"qmi_client_get_service_list rc: %d, num_entries: %d, num_services: %d\n", rc, num_entries, num_services);
    if(rc != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "get service list failed\n");
        return ERR_QMI_GET_SERVICE_LIST;
    }

    rc = qmi_client_init(&info[0], remote_uim_service_object, remote_uim_ind_cb, NULL, NULL, &rm_uim_client);
    MSG_PRINTF(LOG_INFO,"qmi_client_init rc: %d\n", rc);
    if(rc != RT_SUCCESS) {
        return -1;
    }

    req.event_info.event = UIM_REMOTE_CONNECTION_AVAILABLE_V01;
    req.event_info.slot = slot;

    rc = qmi_client_send_msg_async(rm_uim_client, QMI_UIM_REMOTE_EVENT_REQ_V01, &req, sizeof(req),
                                    &resp, sizeof(resp), remote_uim_async_cb, NULL, &txn);

    MSG_PRINTF(LOG_INFO,"t9x07_insert_card slot:%d, rc:%d\n", slot, rc);
    return rc;
}

int t9x07_swap_card(uim_remote_slot_type_enum_v01 slot)
{
    // TODO: figure out why swap not working...
    t9x07_remove_card(UIM_REMOTE_SLOT_1_V01);
    sleep(1);
    t9x07_insert_card(UIM_REMOTE_SLOT_1_V01);
    return RT_SUCCESS;
}

int t9x07_remove_card(uim_remote_slot_type_enum_v01 slot)
{
    int rc;
    uim_remote_event_req_msg_v01 req = {0};
    uim_remote_event_resp_msg_v01 resp = {0};
    qmi_txn_handle txn;

    if (rm_uim_client == NULL) {
        MSG_PRINTF(LOG_INFO,"UIM_REMOTE_CARD_REMOVED_V01 success slot:%d\n", slot);
        return RT_SUCCESS;
    }
    req.event_info.event = UIM_REMOTE_CARD_REMOVED_V01;
    req.event_info.slot = UIM_REMOTE_SLOT_1_V01;

    rc = qmi_client_send_msg_async(rm_uim_client, QMI_UIM_REMOTE_EVENT_REQ_V01, &req, sizeof(req),
                                    &resp, sizeof(resp), remote_uim_async_cb, NULL, &txn);
    MSG_PRINTF(LOG_INFO,"UIM_REMOTE_CARD_REMOVED_V01 slot:%d, rc:%d\n", slot, rc);

    rc = qmi_client_release(rm_uim_client);
    rm_uim_client = NULL;
    MSG_PRINTF(LOG_INFO,"qmi_client_release client rc: %d\n", rc);

    return rc;
}

int t9x07_reset_card(uim_remote_slot_type_enum_v01 slot)
{
    int rc;
    uim_remote_event_req_msg_v01 req = {0};
    uim_remote_event_resp_msg_v01 resp = {0};
    qmi_txn_handle txn;

    req.event_info.event = UIM_REMOTE_CARD_RESET_V01;
    req.event_info.slot = UIM_REMOTE_SLOT_1_V01;
    req.atr_valid = true;

    // fill ATR
    trigger_reset(req.atr, (uint16_t *)&req.atr_len);
    MSG_INFO_ARRAY("ATR", req.atr, req.atr_len);

    rc = qmi_client_send_msg_async(rm_uim_client, QMI_UIM_REMOTE_EVENT_REQ_V01, &req, sizeof(req),
                                    &resp, sizeof(resp), remote_uim_async_cb, NULL, &txn);
    MSG_PRINTF(LOG_INFO, "UIM_REMOTE_CARD_RESET_V01 slot:%d, rc:%d\n", slot, rc);

    return rc;
}

int t9x07_send_card_error(  uim_remote_slot_type_enum_v01       slot,
                                uint8_t                             error_cause_valid,
                                uim_remote_card_error_type_enum_v01 error_cause)
{
    int rc;
    uim_remote_event_req_msg_v01 req = {0};
    uim_remote_event_resp_msg_v01 resp = {0};
    qmi_txn_handle txn;

    req.event_info.event = UIM_REMOTE_CARD_ERROR_V01;
    req.event_info.event = slot;
    req.error_cause_valid = error_cause_valid;
    req.error_cause = error_cause;

    rc = qmi_client_send_msg_async(rm_uim_client, QMI_UIM_REMOTE_EVENT_REQ_V01, &req, sizeof(req),
                                    &resp, sizeof(resp), remote_uim_async_cb, NULL, &txn);
    MSG_PRINTF(LOG_INFO,"UIM_REMOTE_CARD_ERROR_V01 slot:%d, rc:%d\n", slot, rc);

    return rc;
}
