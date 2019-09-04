
#include "upload.h"
#include "rt_type.h"
#include "cJSON.h"

static cJSON *upload_null_end_packer(const char *tran_id, int32_t status, const cJSON *content)
{
    (void)tran_id;
    (void)status;
    (void)content;

    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return NULL;
}

UPLOAD_CMD_OBJ_INIT(END, upload_null_end_packer);

const upload_cmd_t * upload_cmd_get_end(void)
{
    UPLOAD_CMD_OBJ_EXTERN(END);
}

