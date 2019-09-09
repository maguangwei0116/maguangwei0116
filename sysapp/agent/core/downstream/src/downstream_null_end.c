
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

static int32_t downstream_null_end_parser(const void *in, char *tranId, void **out)
{
    (void)in;
    (void)tranId;
    (void)out;

    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return 0;
}

static int32_t downstream_null_end_handler(const void *in, const char *event, void **out)
{
    (void)in;
    (void)event;
    (void)out;

    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return 0;
}

DOWNSTREAM_METHOD_OBJ_INIT(END, MSG_ID_IDLE, END, downstream_null_end_parser, downstream_null_end_handler);

DOWNSTREAM_METHOD_OBJ_EXTERN(END);
