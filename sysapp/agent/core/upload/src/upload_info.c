
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

extern cJSON *upload_event_boot_info(const char *str_event, rt_bool only_profile_network);

static cJSON *upload_info_packer(void *arg)
{
    rt_bool *report_all_info = (rt_bool *)arg;

    if (!report_all_info || *report_all_info == RT_TRUE) {  // report all info
        return upload_event_boot_info("INFO", RT_FALSE);
    } else {
        return upload_event_boot_info("INFO", RT_TRUE);
    }
}

UPLOAD_EVENT_OBJ_INIT(INFO, upload_info_packer);

