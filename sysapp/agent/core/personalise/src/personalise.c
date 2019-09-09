
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

int32_t init_personalise(void *arg)
{
    rt_bool report_all_info;
    MSG_PRINTF(LOG_WARN, "0------------\n");
    upload_event_report("NO_CERT", NULL, 0, NULL);
    MSG_PRINTF(LOG_WARN, "---------report_all_info:%s\n",report_all_info);
    return 0;
}

