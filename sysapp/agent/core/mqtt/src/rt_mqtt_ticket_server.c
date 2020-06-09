
#include "rt_type.h"
#include "cJSON.h"
#include "rt_mqtt.h"
#include "usrdata.h"

/* save cache ticket server which got from adapter into cache file */
rt_bool mqtt_save_ticket_server(const mqtt_opts_t *opts)
{
    cJSON   *obj = NULL;
    uint8_t  *save_info = NULL;
    uint8_t  data_len[2] = {0}; // frist 2 byte store length !!!
    int16_t length = 0;
    rt_bool ret = RT_FALSE;

    if ((obj = cJSON_CreateObject()) == NULL) {
        MSG_PRINTF(LOG_WARN, "cJSON_CreateObject error\n");
        goto exit_entry;
    }

    cJSON_AddItemToObject(obj, "channel", cJSON_CreateString(opts->channel));
    cJSON_AddItemToObject(obj, "ticketServer", cJSON_CreateString(opts->ticket_server));
    save_info = (uint8_t *)cJSON_PrintUnformatted(obj);
    if (!save_info) {
        MSG_PRINTF(LOG_WARN, "save_info is NULL\n");
        goto exit_entry;
    }

    length = rt_os_strlen(save_info);
    data_len[0] = (length >> 8) & 0xff;
    data_len[1] = length & 0xff;

    if (rt_write_ticket(0, data_len, sizeof(data_len)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt write_data data_len error\n");
        goto exit_entry;
    }

    if (rt_write_ticket(sizeof(data_len), save_info, rt_os_strlen(save_info)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt write_data TICKET_SERVER_CACHE error\n");
        goto exit_entry;
    }

    ret = RT_TRUE;

exit_entry:

    if (save_info) {
        cJSON_free(save_info);
    }
    if (obj) {
        cJSON_Delete(obj);
    }

    return ret;
}

/* get cache ticket server which got from adapter from cache file */
rt_bool mqtt_get_ticket_server(mqtt_opts_t *opts)
{
    uint8_t  *save_info = NULL;
    uint8_t  data_len[2] = {0};  // frist 2 byte store length !!!
    int16_t length = 0;
    cJSON   *obj = NULL;
    cJSON   *channel = NULL;
    cJSON   *ticket_server = NULL;
    rt_bool ret = RT_FALSE;

    if (rt_read_ticket(0, data_len, sizeof(data_len)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt read_data data_len error\n");
        goto exit_entry;
    }

    length = (data_len[0] << 8) | data_len[1];

    if ((save_info = (uint8_t *)rt_os_malloc(length)) == NULL) {
        MSG_PRINTF(LOG_ERR, "rt_os_malloc error\n");
        goto exit_entry;
    }

    if (rt_read_ticket(sizeof(data_len), save_info, length) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt read_data TICKET_SERVER_CACHE error\n");
        goto exit_entry;
    }

    if ((obj = cJSON_Parse(save_info)) == NULL) {
        MSG_PRINTF(LOG_WARN, "cJSON_Parse error\n");
        goto exit_entry;
    }
    channel = cJSON_GetObjectItem(obj, "channel");
    ticket_server = cJSON_GetObjectItem(obj, "ticketServer");

    if (!channel || !ticket_server) {
        MSG_PRINTF(LOG_WARN, "channel or ticket_server is NULL\n");
        goto exit_entry;
    }

    snprintf((char *)opts->channel, sizeof(opts->channel), "%s", channel->valuestring);
    snprintf((char *)opts->ticket_server, sizeof(opts->ticket_server), "%s", ticket_server->valuestring);

    ret = RT_TRUE;

exit_entry:

    if (save_info) {
        rt_os_free(save_info);
    }
    if (obj) {
        cJSON_Delete(obj);
    }

    return ret;
}
