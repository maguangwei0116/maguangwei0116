
#include "upload.h"
#include "rt_type.h"
#include "cJSON.h"

static cJSON *upload_null_start_packer(const char *tran_id, int32_t status, const cJSON *content)
{
    (void)tran_id;
    (void)status;
    (void)content;

    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return NULL;
}

UPLOAD_CMD_OBJ_INIT(START, upload_null_start_packer);

const upload_cmd_t *upload_cmd_get_start(void)
{
    UPLOAD_CMD_OBJ_EXTERN(START);
}

