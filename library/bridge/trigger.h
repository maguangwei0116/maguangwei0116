#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include <stdbool.h>
#include "rt_type.h"
// 9x07
#include "user_identity_module_remote_v01.h"

extern int t9x07_insert_card(uim_remote_slot_type_enum_v01 slot);
extern int t9x07_swap_card(uim_remote_slot_type_enum_v01 slot);
extern int t9x07_remove_card(uim_remote_slot_type_enum_v01 slot);

#define trigger_insert_card(slot)              t9x07_insert_card(slot)
#define trigger_swap_card(slot)                t9x07_swap_card(slot)
#define trigger_remove_card(slot)              t9x07_remove_card(slot)

#endif  // __TRIGGER_H__
