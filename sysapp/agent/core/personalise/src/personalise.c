
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : personalise.c
 * Date        : 2019.08.21
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/


#include "rt_type.h"
#include "device_info.h"
#include "upload.h"

int32_t personalise_main(void *arg)
{
    int32_t ret;
    MSG_PRINTF(LOG_WARN, "0------------\n", ret);
    ret = upload_cmd_no_cert(NULL);
    MSG_PRINTF(LOG_WARN, "---------ret:%d\n", ret);
    return ret;
}

