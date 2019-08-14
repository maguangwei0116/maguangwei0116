/*
 * qmi_uim.h
 *
 *  Created on: Aug 29, 2018
 *      Author: ryan
 */

#ifndef QMI_UIM_H_
#define QMI_UIM_H_

#include <qmi-framework/qmi_client.h>
#include <qmi/user_identity_module_v01.h>
#include <stdint.h>

int qmi_wds_init(void);
int qmi_get_elementary_iccid_file(uint8_t *iccid);
int qmi_get_elementary_imsi_file(uint8_t *imsi);

#endif // QMI_UIM_H_
