#ifndef __LDSD_H__
#define __LDSD_H__

#include "rt_type.h"

int get_euicc_configured_address(uint8_t *addr, uint16_t *size);
int set_default_dp_address(char *addr, uint8_t *out, uint16_t *out_size);

#endif  // __LDSD_H__
