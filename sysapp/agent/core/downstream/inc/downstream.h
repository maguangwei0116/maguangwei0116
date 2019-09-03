
#ifndef __DOWNSTREAM_H__
#define __DOWNSTREAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rt_type.h"
#include "cJSON.h"

#define MAX_TRAN_ID_LEN     32

typedef struct _downstram_data_t {
    char            tranId[MAX_TRAN_ID_LEN + 1];
    uint64_t        timestamp;
    uint16_t        expireTime;
    cJSON *         payload;
} downstram_data_t ;

typedef int32_t (*parser)(const char *msg);

typedef struct _downstream_cmd_t {
    const char *    cmd;
    parser *        parser;
} downstream_cmd_t;

#define DOWNSTREAM_CMD_OBJ_INIT(cmd, parser)\
    static const downstream_cmd_t downstream_cmd_##cmd##obj \
    __attribute__((section(".downstream.cmd.init.obj"))) = \
    {#cmd, parser}

int32_t downstram_msg_parse(const char *msg);

#ifdef __cplusplus
}
#endif

#endif /* __DOWNSTRAM_H__*/

