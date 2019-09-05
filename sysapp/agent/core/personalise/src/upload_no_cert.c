
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

static cJSON *upload_nocert_packer(void *arg)
{
    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return NULL;
}

UPLOAD_EVENT_OBJ_INIT(NO_CERT, upload_nocert_packer);

