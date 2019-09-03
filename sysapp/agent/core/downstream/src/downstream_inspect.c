
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

#define DOWNSTRAM_METHOD_STR        "\"method\""
#define DOWNSTRAM_METHOD_STRLEN     8

static int32_t downstram_inpsect_parse(const char *msg)
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

DOWNSTREAM_CMD_OBJ_INIT(INSPECT, downstram_inpsect_parse);


