
/******************************************************************************
  @file    api.h
  @brief   The esim app api include file.

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2018 Redtea Technologies, Inc. All Rights Reserved.
  Redtea Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#ifndef __API_H__
#define __API_H__

#include "rt_type.h"

int32_t softsim_logic_start(void *fun);

uint16_t softsim_logic_command(
    uint8_t slot,
    uint8_t *apdu_req_data,
    uint16_t apdu_req_size,
    uint8_t *apdu_rsp_data,
    uint16_t *apdu_rsp_size);

#endif  // __API_H__
