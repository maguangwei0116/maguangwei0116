
#include "upload.h"
#include "rt_type.h"
#include "cJSON.h"

static cJSON *upload_null_end_packer(void *arg)
{
    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return NULL;
}

UPLOAD_EVENT_OBJ_INIT(END, upload_null_end_packer);

UPLOAD_EVENT_OBJ_EXTERN(END);

