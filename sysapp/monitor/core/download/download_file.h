
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : download_file.h
 * Date        : 2019.10.29
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __DOWNLOAD_FILE_H__
#define __DOWNLOAD_FILE_H__

#include "rt_type.h"

#define MAX_FILE_NAME_LEN               128

typedef struct UPGRADE_STRUCT {
    uint8_t file_name[MAX_FILE_NAME_LEN + 1];
    uint8_t real_file_name[MAX_FILE_NAME_LEN + 1];
} upgrade_struct_t;

int32_t download_start(void);
void init_download(void *args);

#endif  // __DOWNLOAD_FILE_H__
