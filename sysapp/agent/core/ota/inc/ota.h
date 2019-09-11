
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ota.h
 * Date        : 2019.08.21
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __OTA_H__
#define __OTA_H__

int32_t ota_upgrade_event(const uint8_t *buf, int32_t len, int32_t mode);
int32_t init_ota(void *arg);
int32_t ota_upgrade_task_check_event(const uint8_t *buf, int32_t len, int32_t mode);

#endif // __OTA_H__
