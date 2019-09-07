
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ota.c
 * Date        : 2019.08.21
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/
#ifdef CFG_AGENT_OTA_ON
#warning AGENT_OTA_ON on ...

#include "rt_type.h"
#include "ota.h"
#include "downstream.h"

int32_t ota_main(void)
{
    return 0;
}

int32_t ota_upgrade_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    int32_t status = 0;
    downstream_msg_t *downstream_msg = (downstream_msg_t *)buf;

    (void)mode;
    MSG_PRINTF(LOG_INFO, "msg: %s ==> method: %s ==> event: %s\n", downstream_msg->msg, downstream_msg->method, downstream_msg->event);
    
    downstream_msg->parser(downstream_msg->msg, downstream_msg->tranId, &downstream_msg->private_arg);
    if (downstream_msg->msg) {
        rt_os_free(downstream_msg->msg);
        downstream_msg->msg = NULL;
    }
    //MSG_PRINTF(LOG_WARN, "tranId: %s, %p\n", downstream_msg->tranId, downstream_msg->tranId);

    status = downstream_msg->handler(downstream_msg->private_arg, &downstream_msg->out_arg);

    upload_event_report(downstream_msg->event, (const char *)downstream_msg->tranId, status, downstream_msg->out_arg);
}

#endif
