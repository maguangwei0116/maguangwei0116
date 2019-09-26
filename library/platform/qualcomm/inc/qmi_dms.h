/*
 * device_info.h
 *
 *  Created on: Aug 28, 2018
 *      Author: ryan
 */

#ifndef QMI_DMS_H_
#define QMI_DMS_H_

#include <qmi-framework/qmi_client.h>
#include <qmi/device_management_service_v01.h>

typedef struct QMI_DEVICE_INFO {
    /*  Device Manufacturer */
    char device_manufacturer[QMI_DMS_DEVICE_MANUFACTURER_MAX_V01 + 1];

    /*  Device Model */
    char device_model_id[QMI_DMS_DEVICE_MODEL_ID_MAX_V01 + 1];

    /*  Revision ID   */
    char device_rev_id[QMI_DMS_DEVICE_REV_ID_MAX_V01 + 1];

    /*  Boot Code Revision */
    uint8_t boot_code_rev_valid;  /**< Must be set to true if boot_code_rev is being passed */
    char boot_code_rev[QMI_DMS_BOOT_CODE_REV_MAX_V01 + 1];

    /*  Voice Number */
    char voice_number[QMI_DMS_VOICE_NUMBER_MAX_V01 + 1];

    /*  Mobile ID */
    uint8_t mobile_id_number_valid;  /**< Must be set to true if mobile_id_number is being passed */
    char mobile_id_number[QMI_DMS_MOBILE_ID_NUMBER_MAX_V01 + 1];

    /*  International Mobile Subscriber ID */
    uint8_t imsi_valid;  /**< Must be set to true if imsi is being passed */
    char imsi[QMI_DMS_IMSI_MAX_V01 + 1];

    /*  IMEI */
    uint8_t imei_valid;  /**< Must be set to true if imei is being passed */
    char imei[QMI_DMS_IMEI_MAX_V01 + 1];
} qmi_device_info_t;

int qmi_query_device_info(qmi_device_info_t *devinfo);
void dump_device_info(qmi_device_info_t *devinfo);
#endif // QMI_DMS_H_
