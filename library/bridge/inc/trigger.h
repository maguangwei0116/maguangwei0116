#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include <stdbool.h>

#include "vsim_config.h"

#if PLATFORM == PLATFORM_INTEL_X86 || PLATFORM == PLATFORM_ANDROID
// Ubuntu
#define UIM_REMOTE_SLOT_1_V01                   1
#define trigger_insert_card(slot,iccid)        -1
#define trigger_swap_card(slot, iccid)         0
#define trigger_remove_card(slot)              0

#elif PLATFORM == PLATFORM_QCOM_9X07
// 9x07
#include "user_identity_module_remote_v01.h"

extern int t9x07_insert_card(uim_remote_slot_type_enum_v01 slot, char *iccid);
extern int t9x07_swap_card(uim_remote_slot_type_enum_v01 slot, char *iccid);
extern int t9x07_remove_card(uim_remote_slot_type_enum_v01 slot);

#define trigger_insert_card(slot,iccid)        t9x07_insert_card(slot,iccid)
#define trigger_swap_card(slot, iccid)         t9x07_swap_card(slot, iccid)
#define trigger_remove_card(slot)              t9x07_remove_card(slot)

#elif PLATFORM == PLATFORM_MT2503
// 2503
#include "mt2503.h"

extern void esim_driver_init(void);
extern int t2503_insert_card(int slot, char *iccid);
extern int t2503_swap_card(int slot, char *iccid);
extern int t2503_remove_card(int slot);

#define trigger_insert_card(slot,iccid)        t2503_insert_card(slot,iccid)
#define trigger_swap_card(slot, iccid)         t2503_swap_card(slot, iccid)
#define trigger_remove_card(slot)              t2503_remove_card(slot)

#define UIM_REMOTE_SLOT_1_V01                   1

#elif PLATFORM == PLATFORM_MT2625
// 2625
#include "dcl_sim.h"

extern void esim_driver_init(void);
extern int t2625_insert_card(int slot, char *iccid);
extern int t2625_swap_card(int slot, char *iccid);
extern int t2625_remove_card(int slot);

#define trigger_insert_card(slot,iccid)        t2625_insert_card(slot,iccid)
#define trigger_swap_card(slot, iccid)         t2625_swap_card(slot, iccid)
#define trigger_remove_card(slot)              t2625_remove_card(slot)

#define UIM_REMOTE_SLOT_1_V01                   1

#else
#error "Please figure out PLATFORM"
#endif  // PLATFORM

#endif  // __TRIGGER_H__
