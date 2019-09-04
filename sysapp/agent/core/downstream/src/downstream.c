
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

#define DOWNSTRAM_METHOD_STR        "\"method\""
#define DOWNSTRAM_METHOD_STRLEN     8

static int32_t downstram_get_cmd_id(const char *msg)
{
    int32_t id = -1;
    char cmd_id_str[64] = {0};
    const char *p = NULL;
    const char *p0 = NULL;
    const char *p1 = NULL;

    p = rt_os_strstr(msg, DOWNSTRAM_METHOD_STR);
    if (!p) {
        MSG_PRINTF(LOG_WARN, "get cmd id failed\n");
        goto exit_entry;
    }
    p += DOWNSTRAM_METHOD_STRLEN;

    p0 = rt_os_strstr(p, "\"");
    if (!p0) {
        MSG_PRINTF(LOG_WARN, "get cmd id failed\n");
        goto exit_entry;
    }
    
    p1 = rt_os_strstr(p0 + 1, "\"");
    if (!p1) {
        MSG_PRINTF(LOG_WARN, "get cmd id failed\n");
        goto exit_entry;
    }

    rt_os_memcpy(cmd_id_str, p0, p1 - p0);

    id = atoi(cmd_id_str);
    MSG_PRINTF(LOG_INFO, "get cmd id : %d (%s)\n", id, cmd_id_str);

exit_entry:

    return id;
}

int32_t downstram_msg_parse(const char *msg)
{
    int32_t ret = -1;
    int32_t id;

    id = downstram_get_cmd_id(msg);

exit_entry:

    return ret;
}

