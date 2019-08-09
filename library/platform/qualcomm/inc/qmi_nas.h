/*
 * qmi_nas.h
 *
 *  Created on: Aug 29, 2018
 *      Author: ryan
 */

#ifndef QMI_NAS_H_
#define QMI_NAS_H_

#include <qmi-framework/qmi_client.h>
#include <qmi/network_access_service_v01.h>

typedef enum {
    QMI_NAS_NETWORK_IN_USE_STATUS_UNKNOWN = 0,
    QMI_NAS_NETWORK_IN_USE_STATUS_CURRENT_SERVING,
    QMI_NAS_NETWORK_IN_USE_STATUS_AVAILABLE,
} qmi_nas_network_in_use_status_t;

typedef enum {
    QMI_NAS_NETWORK_ROAMING_STATUS_UNKNOWN = 0,
    QMI_NAS_NETWORK_ROAMING_STATUS_HOME,
    QMI_NAS_NETWORK_ROAMING_STATUS_ROAM,
} qmi_nas_network_roaming_status_t;

typedef enum {
    QMI_NAS_NETWORK_FORBIDDEN_STATUS_UNKNOWN = 0,
    QMI_NAS_NETWORK_FORBIDDEN_STATUS_FORBIDDEN,
    QMI_NAS_NETWORK_FORBIDDEN_STATUS_NOT_FORBIDDEN,
} qmi_nas_network_forbidden_status_t;

typedef enum {
    QMI_NAS_NETWORK_PREFERRED_STATUS_UNKNOWN = 0,
    QMI_NAS_NETWORK_PREFERRED_STATUS_PREFERRED,
    QMI_NAS_NETWORK_PREFERRED_STATUS_NOT_PREFERRED,
} qmi_nas_network_preferred_status_t;

typedef struct nas_3gpp_network_info {
    uint16_t mobile_country_code;
    /**<   A 16-bit integer representation of MCC. Range: 0 to 999.  */

    uint16_t mobile_network_code;
    /**<   A 16-bit integer representation of MNC. Range: 0 to 999.  */

    qmi_nas_network_in_use_status_t in_use_status;
    qmi_nas_network_roaming_status_t roaming_status;
    qmi_nas_network_forbidden_status_t forbidden_status;
    qmi_nas_network_preferred_status_t preferred_status;

    char network_description[NAS_NETWORK_DESCRIPTION_MAX_V01 + 1];
    /**<   An optional string containing the network name or description.  */
} nas_3gpp_network_info_t;

typedef struct qmi_network_scan_result {
    uint32_t n_networks;    // Number of networks found
    nas_3gpp_network_info_t *network_info[NAS_3GPP_NETWORK_INFO_LIST_MAX_V01];  // Network information, allocated by scanner
} qmi_network_scan_result;

int qmi_perform_network_scan(qmi_network_scan_result *scan_result, int timeout);
void qmi_network_scan_result_dump(qmi_network_scan_result *scan_result);
void qmi_network_scan_result_free(qmi_network_scan_result *scan_result);

typedef struct qmi_serving_system_info {
    nas_serving_system_type_v01 serving_system;
    /* Optional */
    /*  Roaming Indicator Value */
    nas_roaming_indicator_enum_v01 roaming_indicator;
    nas_mnc_pcs_digit_include_status_type_v01 mnc_includes_pcs_digit;
} qmi_serving_system_info_t;

int qmi_get_serving_system(qmi_serving_system_info_t *info);
void qmi_serving_system_dump(qmi_serving_system_info_t *info);

typedef struct qmi_signal_strength_info {
    int signal_strength;
    uint16_t error_rate;
} qmi_signal_strength_info_t;

int qmi_get_signal_strength(qmi_signal_strength_info_t *info);
int qmi_nas_init(void);
#endif // QMI_NAS_H_
