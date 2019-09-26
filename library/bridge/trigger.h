#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include <stdbool.h>
#include "rt_type.h"
// 9x07
#include "user_identity_module_remote_v01.h"

typedef enum TRIGGER_ERROR_CODE {
    ERR_QMI_RUIM_SERVICE_OBJ = 1,
    ERR_QMI_UNSUPPORTED_SLOT,
    ERR_QMI_GET_SERVICE_LIST
} trigger_error_code_e;

extern int t9x07_insert_card(uim_remote_slot_type_enum_v01 slot);
extern int t9x07_swap_card(uim_remote_slot_type_enum_v01 slot);
extern int t9x07_remove_card(uim_remote_slot_type_enum_v01 slot);

#define trigger_insert_card(slot)              t9x07_insert_card(slot)
#define trigger_swap_card(slot)                t9x07_swap_card(slot)
#define trigger_remove_card(slot)              t9x07_remove_card(slot)
#define trigger_reset_card(slot)               t9x07_reset_card(slot)

void trigegr_regist_reset(void *fun);
void trigegr_regist_cmd(void *fun);
#endif  // __TRIGGER_H__
