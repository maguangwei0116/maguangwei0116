
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

static cJSON *upload_registered_packer(const char *tran_id, int32_t status, const cJSON *content)
{
    (void)tran_id;
    (void)status;
    (void)content;

    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return NULL;
}

UPLOAD_CMD_OBJ_INIT(REGISTERED, upload_registered_packer);

