

#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

static int32_t downstream_null_start_parser(const void *in, char *tranId, void **out)
{
    int32_t id = -1;
    char cmd_id_str[64] = {0};
    const char *p = NULL;
    const char *p0 = NULL;
    const char *p1 = NULL;

    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return id;
}

static int32_t downstream_null_start_handler(const void *in, void **out)
{
    int32_t id = -1;
    char cmd_id_str[64] = {0};
    const char *p = NULL;
    const char *p0 = NULL;
    const char *p1 = NULL;

    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return id;
}

DOWNSTREAM_METHOD_OBJ_INIT(START, MSG_ID_IDLE, START, downstream_null_start_parser, downstream_null_start_handler);

DOWNSTREAM_METHOD_OBJ_EXTERN(START);

