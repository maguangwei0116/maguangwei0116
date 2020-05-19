

#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

static int32_t downstream_null_start_parser(const void *in, char *tranId, void **out)
{
    (void)in;
    (void)tranId;
    (void)out;

    MSG_PRINTF(LOG_INFO, "\n");

exit_entry:

    return RT_SUCCESS;
}

static int32_t downstream_null_start_handler(const void *in, const char *event, void **out)
{
    (void)in;
    (void)event;
    (void)out;

    MSG_PRINTF(LOG_INFO, "\n");

exit_entry:

    return RT_SUCCESS;
}

DOWNSTREAM_METHOD_OBJ_INIT(START, MSG_ID_IDLE, START, downstream_null_start_parser, downstream_null_start_handler);

DOWNSTREAM_METHOD_OBJ_EXTERN(START);

