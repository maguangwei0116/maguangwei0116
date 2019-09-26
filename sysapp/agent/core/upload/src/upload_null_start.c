
#include "upload.h"
#include "rt_type.h"
#include "cJSON.h"

static cJSON *upload_null_start_packer(void *arg)
{
    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return NULL;
}

UPLOAD_EVENT_OBJ_INIT(START, TOPIC_DEVICEID_OR_EID, upload_null_start_packer);

UPLOAD_EVENT_OBJ_EXTERN(START);

