#ifndef __QMI_WDS_H_
#define __QMI_WDS_H_

#include <qmi-framework/qmi_client.h>
#include <qmi/wireless_data_service_v01.h>

typedef struct qmi_wds_profile_info {
    int profile_type;
    /*<   Identifies the type of the profile. Values: \n
          - WDS_PROFILE_TYPE_3GPP (0x00) --  3GPP \n
          - WDS_PROFILE_TYPE_3GPP2 (0x01) --  3GPP2 \n
          - WDS_PROFILE_TYPE_EPC (0x02) --  EPC
     */
    char profile_index;
    char *apn_name;
    char *usrname;
    char *passwd;
    int  pdp_type;
      /**<   Values: \n
           - 0 -- IPv4 PDN type \n
           - 1 -- IPv6 PDN type \n
           - 2 -- IPv4 or IPv6 PDN type \n
           - 3 -- Unspecified PDN type (no preference)
       */
} qmi_wds_profile_info_t;
int qmi_modify_profile(qmi_wds_profile_info_t *info);

#endif  // __QMI_WDS_H_
