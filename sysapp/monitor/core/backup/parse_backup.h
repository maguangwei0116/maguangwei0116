
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : profile_parse.h
 * Date        : 2019.08.28
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __PARSE_BACKUP_H__
#define __PARSE_BACKUP_H__

#include "rt_type.h"

typedef enum LPA_CHANNEL_TYPE {
    LPA_CHANNEL_BY_IPC = 0,
    LPA_CHANNEL_BY_QMI
} lpa_channel_type_e;

void init_apdu_channel(lpa_channel_type_e channel_mode);
int32_t backup_process(void);

#endif // __PARSE_BACKUP_H__
