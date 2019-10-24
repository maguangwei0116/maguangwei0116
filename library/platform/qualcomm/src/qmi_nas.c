#include "qmi_nas.h"
#include "qmi_control_point.h"

#include <string.h>
#include <stdlib.h>

static qmi_client_type g_nas_client;

int qmi_nas_init(void)
{
    qmi_client_error_type err;

    err = qmi_ctrl_point_init(nas_get_service_object_v01(), &g_nas_client, NULL, NULL);
    if(err != QMI_NO_ERR) {
        MSG_PRINTF(LOG_WARN, "failed to initialize control point of NAS: %d\n", err);
    }

    return err;
}

int qmi_perform_network_scan(qmi_network_scan_result *scan_result, int timeout)
{
    qmi_client_error_type err;
    int n, i;

    scan_result->n_networks = 0;
    memset(scan_result->network_info, 0, sizeof(scan_result->network_info));

    nas_perform_network_scan_req_msg_v01 req = { 0 };
    nas_perform_network_scan_resp_msg_v01 resp = { 0 };

    QMI_CLIENT_SEND_SYNC_TMO(err, g_nas_client, QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01, req, resp, timeout);
    if(err == QMI_NO_ERR) {
        if(resp.resp.result != QMI_RESULT_SUCCESS_V01) {
            MSG_PRINTF(LOG_WARN, "network scan failed, result: %d, error code: %d\n", resp.resp.result, resp.resp.error);
            err = resp.resp.result;
            goto out;
        }
        if(!resp.nas_3gpp_network_info_valid) {
            MSG_PRINTF(LOG_WARN, "network scan got none 3GPP network\n");
            err = QMI_NO_ERR;
            goto out;
        }
        n = resp.nas_3gpp_network_info_len;
        if(n < 0 || n > NAS_3GPP_NETWORK_INFO_LIST_MAX_V01) {
            MSG_PRINTF(LOG_WARN, "invalid network number: %d\n", n);
            err = -100;
            goto out;
        }
        scan_result->n_networks = n;
        for(i = 0; i < n; i ++) {
            scan_result->network_info[i] = (nas_3gpp_network_info_t *)calloc(1, sizeof(nas_3gpp_network_info_t));
            nas_3gpp_network_info_t *info = scan_result->network_info[i];
            info->mobile_country_code = resp.nas_3gpp_network_info[i].mobile_country_code;
            info->mobile_network_code = resp.nas_3gpp_network_info[i].mobile_network_code;
            strcpy(info->network_description, resp.nas_3gpp_network_info[i].network_description);

            info->in_use_status = (resp.nas_3gpp_network_info[i].network_status) & 3;
            info->roaming_status = (resp.nas_3gpp_network_info[i].network_status >> 2) & 3;
            info->forbidden_status = (resp.nas_3gpp_network_info[i].network_status >> 4) & 3;
            info->preferred_status = (resp.nas_3gpp_network_info[i].network_status >> 6) & 3;
        }
    }
out:

    return err;
}

void qmi_network_scan_result_dump(qmi_network_scan_result *scan_result)
{
    int i;
    for(i = 0; i < scan_result->n_networks; i ++) {
        nas_3gpp_network_info_t *info = scan_result->network_info[i];
        MSG_PRINTF(LOG_INFO, "Scanned Network[%d] MCC:%03d MNC:%03d IN-USE:%d ROAMING:%d FORBIDDEN:%d PREFERRED:%d DESC:'%s'\n",
                i + 1,
                info->mobile_country_code,
                info->mobile_network_code,
                info->in_use_status,
                info->roaming_status,
                info->forbidden_status,
                info->preferred_status,
                info->network_description);
    }
}

void qmi_network_scan_result_free(qmi_network_scan_result *scan_result)
{
    int i;
    for(i = 0; i < scan_result->n_networks; i ++)
        if(scan_result->network_info[i]) {
            free(scan_result->network_info[i]);
            scan_result->network_info[i] = NULL;
        }
}

int qmi_get_serving_system(qmi_serving_system_info_t *info)
{
    qmi_client_error_type err;

    memset(info, 0, sizeof(qmi_serving_system_info_t));

    nas_get_serving_system_req_msg_v01 req = { 0 };
    nas_get_serving_system_resp_msg_v01 resp = { 0 };

    QMI_CLIENT_SEND_SYNC(err, g_nas_client, QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01, req, resp);
    if(err == QMI_NO_ERR) {
        if(resp.resp.result != QMI_RESULT_SUCCESS_V01) {
            MSG_PRINTF(LOG_WARN, "get serving system failed, result: %d, error code: %d\n", resp.resp.result, resp.resp.error);
            err = resp.resp.result;
            goto out;
        }
        memcpy(&info->serving_system, &resp.serving_system, sizeof(info->serving_system));
        if(resp.roaming_indicator_valid) {
            info->roaming_indicator = resp.roaming_indicator;
        }
        if(resp.mnc_includes_pcs_digit_valid) {
             memcpy(&info->mnc_includes_pcs_digit, &resp.mnc_includes_pcs_digit, sizeof(info->mnc_includes_pcs_digit));
        }

    }

out:
    return err;
}

void qmi_serving_system_dump(qmi_serving_system_info_t *info)
{
    static const char *register_str[] = {
            "Not registered, not searching",
            "Registered",
            "Not registered, searching",
            "Registration denied",
            "Unknown",
    };

    static const char *attach_str[] = {
            "Unknown",
            "Attached",
            "Detached",
    };

    static const char *selected_network_str[] = {
            "Unknown",
            "3GPP2",
            "3GPP",
    };

    static const char *radio_str[] = {
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

    MSG_PRINTF(LOG_WARN, "Registration State: %s\n",
            info->serving_system.registration_state <= 4 ?
            register_str[info->serving_system.registration_state] : "Invalid value");

    MSG_PRINTF(LOG_WARN, "Circuit switch domain attach state: %s\n",
            info->serving_system.cs_attach_state <= 2 ?
            attach_str[info->serving_system.cs_attach_state] : "Invalid value");

    MSG_PRINTF(LOG_WARN, "Packet switch domain attach state: %s\n",
            info->serving_system.ps_attach_state <= 2 ?
            attach_str[info->serving_system.ps_attach_state] : "Invalid value");

    MSG_PRINTF(LOG_WARN, "Registered Network: %s\n",
            info->serving_system.selected_network <= 2 ?
            selected_network_str[info->serving_system.selected_network] : "Invalid value");

    int i;
    for(i = 0; i < info->serving_system.radio_if_len; i ++) {
        MSG_PRINTF(LOG_WARN, "Radio Interface %d: %s\n", i + 1,
                info->serving_system.radio_if[i] <= 9 ?
                radio_str[info->serving_system.radio_if[i]] : "Invalid value");
    }
}

int qmi_get_signal_strength(qmi_signal_strength_info_t *info)
{
    qmi_client_error_type err;

    memset(info, 0, sizeof(qmi_signal_strength_info_t));
    info->error_rate = 0xffff;

    nas_get_signal_strength_req_msg_v01 req = {
            .request_mask_valid = 1,
            .request_mask = QMI_NAS_REQUEST_SIG_INFO_ERROR_RATE_MASK_V01,
    };
    nas_get_signal_strength_resp_msg_v01 resp = { 0 };

    QMI_CLIENT_SEND_SYNC(err, g_nas_client, QMI_NAS_GET_SIGNAL_STRENGTH_REQ_MSG_V01, req, resp);
    if(err == QMI_NO_ERR) {
        if(resp.resp.result != QMI_RESULT_SUCCESS_V01) {
            MSG_PRINTF(LOG_ERR, "get signal strength failed, result: %d, error code: %d\n", resp.resp.result, resp.resp.error);
            err = resp.resp.result;
            goto out;
        }

        info->signal_strength = resp.signal_strength.sig_strength;
        if(resp.error_rate_valid) {
            // FIXME: Only the first error rate is used
            info->error_rate = resp.error_rate[0].error_rate;
        }
    }

out:
    return err;
}
