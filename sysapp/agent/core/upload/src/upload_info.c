
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

extern cJSON *upload_event_boot_info(const char *str_event, rt_bool only_profile_network);

static cJSON *upload_info_packer(void *arg)
{
    return upload_event_boot_info("INFO", RT_TRUE);
}

UPLOAD_EVENT_OBJ_INIT(INFO, upload_info_packer);

