#include "ldsd.h"
#include "lpdd.h"
#include "apdu.h"
#include "lpa_config.h"
#include "lpa_error_codes.h"
#include "bertlv.h"

// Defined in lpdd.c
extern int encode_cb(const void *buffer, size_t size, void *app_key);
extern void clean_cb_data(void);
extern uint8_t *get_cb_data(void);
extern uint16_t get_cb_size(void);

int get_euicc_configured_address(uint8_t *addr, uint16_t *size, uint8_t channel)
{
    /*
    EuiccConfiguredAddressesRequest ::= [60] SEQUENCE { -- Tag 'BF3C' }
    */

    clean_cb_data();
    g_buf_size = bertlv_build_tlv(TAG_LPA_GET_EUICC_CONFGI_ADDR_REQ, 0, NULL, g_buf);
    
    MSG_DUMP_ARRAY("EuiccConfiguredAddressesRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), addr, size, channel));

    return RT_SUCCESS;
}

int set_default_dp_address(char *addr, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    /*
    SetDefaultDpAddressRequest ::= [63] SEQUENCE { -- Tag 'BF3F' 
        defaultDpAddress UTF8String -- Default SM-DP+ address as an FQDN 
    }
    */

    clean_cb_data();
    g_buf_size = bertlv_build_tlv(0x80, strlen(addr), addr, g_buf);
    g_buf_size = bertlv_build_tlv(TAG_LPA_SET_DEFAULT_DP_ADDR_REQ, g_buf_size, g_buf, g_buf);

    MSG_DUMP_ARRAY("EuiccConfiguredAddressesRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel));

    return RT_SUCCESS;
}
