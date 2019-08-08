#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include <stdbool.h>
#include "rt_type.h"

#if 0
// Ubuntu
#define UIM_REMOTE_SLOT_1_V01                   1
#define trigger_insert_card(slot,iccid)        -1
#define trigger_swap_card(slot, iccid)         0
#define trigger_remove_card(slot)              0
#endif

#include "user_identity_module_remote_v01.h"

extern int t9x07_insert_card(uim_remote_slot_type_enum_v01 slot, char *iccid);
extern int t9x07_swap_card(uim_remote_slot_type_enum_v01 slot, char *iccid);
extern int t9x07_remove_card(uim_remote_slot_type_enum_v01 slot);

#define trigger_insert_card(slot,iccid)        t9x07_insert_card(slot,iccid)
#define trigger_swap_card(slot, iccid)         t9x07_swap_card(slot, iccid)
#define trigger_remove_card(slot)              t9x07_remove_card(slot)

#else
#error "Please figure out PLATFORM"
#endif  // PLATFORM

#endif  // __TRIGGER_H__
