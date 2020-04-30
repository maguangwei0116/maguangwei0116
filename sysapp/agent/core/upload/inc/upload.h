
#ifndef __NEW_UPLOAD_H__
#define __NEW_UPLOAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http.h"
#include "rt_type.h"
#include "cJSON.h"
#include "https.h"

/* uuid sample: 6c90fcc5-a30d-444f-9ba4-5bc433895699 */
#define MAX_UUID_LEN                37  //(32+4+1)
#define MAX_TRAN_ID_LEN             64

typedef enum UPLOAD_TOPIC {
    TOPIC_DEVICEID                  = 0,
    TOPIC_DEVICEID_OR_EID           = 1,    
} upload_topic_e;

typedef cJSON *(*packer_func)(void *arg);

typedef struct UPLOAD_EVENT {
    const char *    event;
    upload_topic_e  topic;
    packer_func     packer;
} upload_event_t;

#define UPLOAD_EVENT_OBJ_INIT(event, topic, packer)\
    static const upload_event_t upload_event_##event##_obj \
    __attribute__((section(".upload.event.init.obj"))) __attribute__((__used__)) = \
    {#event, topic, packer}

#define UPLOAD_EVENT_OBJ_EXTERN(event) \
    const upload_event_t * g_upload_event_##event = (const upload_event_t * )&upload_event_##event##_obj

#define UPLOAD_EVENT_OBJ_EXTERN_HERE(event) \
    extern const upload_event_t * g_upload_event_##event

UPLOAD_EVENT_OBJ_EXTERN_HERE(START);
UPLOAD_EVENT_OBJ_EXTERN_HERE(END);

int32_t upload_event_report(const char *event, const char *tran_id, int32_t status, void *private_arg);
int32_t upload_event_final_report(void *buffer, int32_t len);
int32_t init_upload(void *arg);
int32_t upload_cmd_no_cert(void *arg);
int32_t upload_event(const uint8_t *buf, int32_t len, int32_t mode);

#ifdef __cplusplus
}
#endif

#endif /* __NEW_UPLOAD_H__*/

