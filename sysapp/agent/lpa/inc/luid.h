#ifndef __LUID_H__
#define __LUID_H__

#include <stdbool.h>
#include "rt_type.h"

typedef enum profile_identifier {
    PID_ISDP_AID,
    PID_ICCID,
} profile_id_t;

typedef enum search_criteria {
    SEARCH_NONE,
    SEARCH_ISDP_AID,
    SEARCH_ICCID,
    SEARCH_PROFILE_CLASS
} search_criteria_t;

typedef enum memory_reset_request {
    RESET_OPERATIONAL_PROFILE   = 0x01,
    RESET_TEST_PROFILE          = 0x02,
    RESET_DEFAULT_SDMP_ADDRESS  = 0x04
} memory_reset_t;

// ES10c
int enable_profile(profile_id_t pid, uint8_t id[16], bool refresh, uint8_t *out, uint16_t *out_size);
int disable_profile(profile_id_t pid, uint8_t id[16], bool refresh, uint8_t *out, uint16_t *out_size);
int delete_profile(profile_id_t pid, uint8_t id[16], uint8_t *out, uint16_t *out_size);
int get_eid(uint8_t *eid, uint16_t *size);
int get_profiles_info(search_criteria_t sc, uint8_t *criteria, uint16_t c_size,
                    uint8_t *profile_info, uint16_t *size /* out */);
int set_nickname(uint8_t iccid[10], const char *nickname, uint8_t *out, uint16_t *out_size);
int euicc_memory_reset(memory_reset_t mrt, uint8_t *out, uint16_t *out_size);
int switch_eid(uint8_t *eid, uint16_t size,uint8_t *out, uint16_t *out_size);
#endif  // __LUID_H__
