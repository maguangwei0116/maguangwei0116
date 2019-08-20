#include "ldsd.h"
#include "apdu.h"
#include "lpa_config.h"

#include "EuiccConfiguredAddressesRequest.h"
#include "SetDefaultDpAddressRequest.h"

// Defined in lpdd.c
extern int encode_cb(const void *buffer, size_t size, void *app_key);
extern void clean_cb_data(void);
extern uint8_t *get_cb_data(void);
extern uint16_t get_cb_size(void);

int get_euicc_configured_address(uint8_t *addr, uint16_t *size)
{
    asn_enc_rval_t ec;
    EuiccConfiguredAddressesRequest_t req = {0};

    clean_cb_data();
    ec = der_encode(&asn_DEF_EuiccConfiguredAddressesRequest, &req, encode_cb, NULL);
    MSG_INFO("ec.encoded: %d\n", (int)ec.encoded);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("EuiccConfiguredAddressesRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), addr, size));
    *size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int set_default_dp_address(const char *addr, uint8_t *out, uint16_t *out_size)
{
    asn_enc_rval_t ec;
    SetDefaultDpAddressRequest_t req = {0};

    req.defaultDpAddress.buf = addr;
    req.defaultDpAddress.size = strlen(addr);

    clean_cb_data();
    ec = der_encode(&asn_DEF_SetDefaultDpAddressRequest, &req, encode_cb, NULL);
    MSG_INFO("ec.encoded: %d\n", (int)ec.encoded);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("EuiccConfiguredAddressesRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size));
    *out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}
