/**
* 对配置文件进行读写操作
* 所有配置以 key - value pair 的形式存储
* 具体连接符和注释标号在 c 文件中定义
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include "rt_type.h"
#include "file.h"
#include "rt_os.h"
#include "log.h"
#include "agent_queue.h"
#include "md5.h"
#include "downstream.h"
#include "upload.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)                       (sizeof((a)) / sizeof((a)[0]))
#endif

#define M_BYTES                             (1024 * 1024)
#define MAX_LINE_SIZE                       384
#define MAX_VALUE_SIZE                      30
#define LINK_SYMBOL                         '='                     // key - value pair 之间的连接符
#define ANNOTATION_SYMBOL                   '#'                     // 注释标识符
#define CONFIG_FILE_PATH                    "/data/redtea/rt_config.ini"
#define IS_SPACES(x)                        ( ' ' == (x) || '\t' == (x) || '\n' == (x) || '\r' == (x) || '\f' == (x) || '\b' == (x) )  // 判定是否为空白符
#define UICC_MODE_vUICC                     "0"
#define UICC_MODE_eUICC                     "1"

typedef int32_t (*config_func)(const void *in, char *out);

typedef struct CONFIG_ITEM {    
    const char *            key;
    char                    value[MAX_VALUE_SIZE];
    config_func             config;
    char *                  def_value;
    const char *            annotation;
} config_item_t;

#define ITEM(value, config, def_value, annotation)\
{\
    #value, {0}, config, def_value, annotation,\
}

static int32_t config_range_int_value(const void *in, int32_t min, int32_t max, char *out)
{
    int32_t int_value = (int32_t)in;
    MSG_PRINTF(LOG_INFO, "%s, int_value=%d\n", __func__, int_value); 

    if (min <= int_value && int_value <= max) {
        sprintf(out, "%d", int_value);
        return RT_SUCCESS;
    } else {
        return RT_ERROR;
    }
}

static int32_t config_string_value(const void *in, char *out)
{
    const char *str_value = (const char *)in;
    MSG_PRINTF(LOG_INFO, "%s, str_value=%s, len=%d\n", __func__, str_value, rt_os_strlen(str_value));   

    if (rt_os_strlen(str_value) > 0) {
        sprintf(out, "%s", str_value);
        return RT_SUCCESS;
    } else {
        return RT_ERROR;
    }
}

/* value: [0,1] */
static int32_t config_switch_value(const void *in, char *out)
{
    int32_t int_value = (int32_t)in;

    return config_range_int_value(in, 0, 1, out);
}

/* value: [LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO] */
static int32_t config_log_level(const void *in, char *out)
{
    const char *str_value = (const char *)in;
    log_level_e level = log_get_level(str_value);

    if (LOG_UNKNOW == level) {
        return RT_ERROR;
    } else {
        return config_string_value(in, out);
    }
}

/* value: [0,2] */
static int32_t config_init_pro_type(const void *in, char *out)
{
    return config_range_int_value(in, 0, 2, out);  
}

/* value: [0,5] */
static int32_t config_log_size(const void *in, char *out)
{
    return config_range_int_value(in, 0, 5, out);  
}

/* value: [0,1] */
static int32_t config_uicc_mode(const void *in, char *out)
{
    return config_range_int_value(in, 0, 1, out);  
}

/* value: [60,1440] (60mins -- 1 hour) */
static int32_t config_usage_freq(const void *in, char *out)
{
    return config_range_int_value(in, 60, 1440, out);  
}

static config_item_t g_config_items[] = 
{
/*     item_name            config_func             default_value           annotation          */
#if (CFG_ENV_TYPE_PROD)
ITEM(OTI_ENVIRONMENT_ADDR,  NULL,                   "52.220.34.227",        "OTI server addr: stage(54.222.248.186) or prod(52.220.34.227)"), 
ITEM(EMQ_SERVER_ADDR,       NULL,                   "18.136.190.97",        "EMQ server addr: stage(13.229.31.234) prod(18.136.190.97)"),
ITEM(PROXY_SERVER_ADDR,     NULL,                   "smdp.redtea.io",       "SMDP server addr: stage(smdp-test.redtea.io) prod(smdp.redtea.io) qa(smdp-test.redtea.io)"),
#else
ITEM(OTI_ENVIRONMENT_ADDR,  config_string_value,                   "54.222.248.186",       "OTI server addr: stage(54.222.248.186) or prod(52.220.34.227)"), 
ITEM(EMQ_SERVER_ADDR,       config_string_value,                   "13.229.31.234",        "EMQ server addr: stage(13.229.31.234) prod(18.136.190.97)"),
ITEM(PROXY_SERVER_ADDR,     config_string_value,    "smdp-test.redtea.io",  "SMDP server addr: stage(smdp-test.redtea.io) prod(smdp.redtea.io) qa(smdp-test.redtea.io)"),
#endif
ITEM(MBN_CONFIGURATION,     config_switch_value,    "1",                    "Whether config MBN (0:disable  1:enable)"),
ITEM(INIT_PROFILE_TYPE,     config_init_pro_type,   "2",                    "The rules of the first boot option profile (0:Provisioning 1:Operational 2:last)"),
ITEM(RPLMN_ENABLE,          config_switch_value,    "1",                    "Whether set rplmn (0:disable  1:enable)"),
ITEM(LOG_FILE_SIZE,         config_log_size,        "1",                    "The max size of rt_log file (MB)"),
ITEM(UICC_MODE,             config_uicc_mode,       "1",                    "The mode of UICC (0:vUICC  1:eUICC)"),
#if (CFG_SOFTWARE_TYPE_RELEASE)
ITEM(MONITOR_LOG_LEVEL,     config_log_level,       "LOG_WARN",             "The log level of monitor (LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO)"),
ITEM(AGENT_LOG_LEVEL,       config_log_level,       "LOG_WARN",             "The log level of agent (LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO)"),
#else
ITEM(MONITOR_LOG_LEVEL,     config_log_level,       "LOG_INFO",             "The log level of monitor (LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO)"),
ITEM(AGENT_LOG_LEVEL,       config_log_level,       "LOG_INFO",             "The log level of agent (LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO)"),
#endif
ITEM(USAGE_ENABLE,          config_switch_value,    "0",                    "Whether enable upload user traffic (0:disable  1:enable)"),
ITEM(USAGE_FREQ,            config_usage_freq,      "60",                   "Frequency of upload user traffic ( 60 <= x <= 1440 Mins)"),
};

static config_info_t g_config_info;

static uint32_t msg_string_to_int(const char *str)
{
    uint32_t length = 0;

    if (!str) {
        return length;
    }
    
    while (*str != '\0') {
        if (('0' <= *str) && (*str <= '9')) {
            length = length * 10 + *str - '0';
        }
        str++;
    }
    
    return length;
}

static int32_t config_set_data(const char *key, const char *value, int32_t pair_num, config_item_t *items)
{
    int32_t i = 0;
    
    for (i = 0; i < pair_num; i++) {
        if (!rt_os_strcmp(items[i].key, key)) {
            snprintf(items[i].value, sizeof(items[i].value), "%s", value);
        }
    }
    
    return RT_SUCCESS;
}

static char *config_get_data(const char *key, int32_t pair_num, const config_item_t *items)
{
    int32_t i = 0;
    
    for (i = 0; i < pair_num; i++) {
        if (!rt_os_strcmp(items[i].key, key)) {
            return (char *)items[i].value;
        }
    }
    
    return "";
}

/* save config file */
static int32_t config_write_file(const char *file, int32_t pair_num, const config_item_t *items)
{
    int32_t i = 0;
    int32_t ret = RT_ERROR;
    rt_fshandle_t fp;
    char item_data[MAX_LINE_SIZE];

    fp = linux_fopen(file, "w");
    if (!fp) {
        goto exit_entry;
    }

    for (; i < pair_num; i++) {
        snprintf(item_data, sizeof(item_data), "%c%s\n%s %c %s\n\n", 
            ANNOTATION_SYMBOL, items[i].annotation, items[i].key, LINK_SYMBOL, items[i].value);
        linux_fwrite(item_data, 1, rt_os_strlen(item_data), fp);
    }

    ret = RT_SUCCESS;

exit_entry:
    
    if (fp) {
        linux_fclose(fp);
        fp = NULL;
    }
    
    return ret;
}

/* trim spaces which in start/end of a tring */
static void config_string_trim(char *str_in, char *str_out)
{
    char *start, *end, *temp;
    
    temp = str_in;
    while (IS_SPACES(*temp)) {
        ++temp;
    }

    start = temp;
    temp = str_in + rt_os_strlen(str_in) - 1;

    while (IS_SPACES(*temp)) {
        --temp;
    }

    end = temp; 
    for(str_in = start; str_in <= end;) {
        *str_out++ = *str_in++;
    }

    *str_out = '\0';
}

static int32_t config_get_key_value(const char *data, char *key, char *value)
{
    const char *p = NULL;
    const char *p0 = data;
    const char *p1 = data;

    if (!(p = rt_os_strchr(data, LINK_SYMBOL))) {
        return RT_FALSE;
    }

    while (!IS_SPACES(*p1)) {
        p1++;
    }
    rt_os_memcpy(key, p0, p1-p0);

    p++;
    p0 = p;
    while (IS_SPACES(*p0)) {
        p0++;
    }

    p1 = p0 + 1;
    while (!IS_SPACES(*p1) && *p1 != '\0') {
        p1++;
    }

    rt_os_memcpy(value, p0, p1-p0);
    //MSG_PRINTF(LOG_INFO, "key=[%s], value=[%s]\r\n", key, value);

    return RT_SUCCESS;
}

static int32_t config_read_file(const char *file, int32_t pair_num, config_item_t *items)
{
    int32_t ret = RT_ERROR;
    rt_fshandle_t fp = NULL;
    int32_t i = 0;    
    char line_data[MAX_LINE_SIZE];
    char line_data_out[MAX_LINE_SIZE];
    char key[MAX_VALUE_SIZE];
    char value[MAX_VALUE_SIZE];

    fp = linux_fopen(file, "r");
    if(!fp) {
        MSG_PRINTF(LOG_WARN, "can't open file %s\n", file);
        goto exit_entry;
    }

    /* get config item line by line */
    while(!linux_feof(fp)) {
        rt_os_memset(line_data, 0, sizeof(line_data));
        rt_os_memset(line_data_out, 0, sizeof(line_data_out));
        if (linux_fgets(line_data, sizeof(line_data), fp) != NULL) {
            /* get one config item in a line */
            config_string_trim(line_data, line_data_out);            
            if (line_data_out[0] == ANNOTATION_SYMBOL) {
                continue;
            }

            /* get config item key and value, and then set then into global variables */
            rt_os_memset(key, 0, sizeof(key));
            rt_os_memset(value, 0, sizeof(value));
            if (RT_SUCCESS == config_get_key_value(line_data_out, key, value)) {
                config_set_data((const char *)key, (const char *)value, pair_num, items);
            }
        }
    }
    
    ret = RT_SUCCESS;

exit_entry:
    
    if (fp) {
        linux_fclose(fp);
        fp = NULL;
    }

    return ret;
}      

/**
* parse config file
*/
static void config_parse_file(const char *file, int32_t pair_num, config_item_t *items)
{
    config_read_file(file, pair_num, items);
}

/**
* update config file
*/
static void config_create_default_file(const char *file, int32_t pair_num, config_item_t *items)
{
    int32_t i = 0;

    /* load default config value */
    for (i = 0; i < pair_num; i++) {
        snprintf(items[i].value, sizeof(items[i].value), "%s", items[i].def_value);
    }

    /* write defconfig file */
    config_write_file(file, pair_num, items);
    MSG_PRINTF(LOG_DBG, "Create default param for [%s] environment !!!\r\n", CFG_ENV_TYPE);
}

static int32_t config_init_file(const char *file, int32_t pair_num, config_item_t *items)
{
    if (rt_os_access(file, RT_FS_F_OK) == RT_ERROR) {
        config_create_default_file(file, pair_num, items);
    }

    config_parse_file(file, pair_num, items);

    return RT_SUCCESS;
}

#define local_config_get_data(key)    config_get_data(key, pair_num, items)

static int32_t config_sync_global_info(config_info_t *infos, int32_t pair_num, const config_item_t *items)
{
    int8_t log_level;
    uint32_t size;
    
    infos->oti_addr          = local_config_get_data("OTI_ENVIRONMENT_ADDR");
    infos->emq_addr          = local_config_get_data("EMQ_SERVER_ADDR");
    infos->proxy_addr        = local_config_get_data("PROXY_SERVER_ADDR");
    infos->lpa_channel_type  = !rt_os_strcmp(local_config_get_data("UICC_MODE"), UICC_MODE_vUICC) ? \
                                                    LPA_CHANNEL_BY_IPC : LPA_CHANNEL_BY_QMI;

    size = msg_string_to_int(local_config_get_data("LOG_FILE_SIZE"));
    if (size > 0) {
        infos->log_max_size = size * M_BYTES;
    }

    log_level = log_get_level(local_config_get_data("MONITOR_LOG_LEVEL"));
    infos->monitor_log_level = (LOG_UNKNOW == log_level) ? LOG_INFO : log_level;

    log_level = log_get_level(local_config_get_data("AGENT_LOG_LEVEL"));
    infos->agent_log_level = (LOG_UNKNOW == log_level) ? LOG_INFO : log_level;

    return RT_SUCCESS;
}

static void config_debug_cur_param(int32_t pair_num, const config_item_t *items)
{
    MSG_PRINTF(LOG_WARN, "Agent version: %s\n", LOCAL_TARGET_RELEASE_VERSION_NAME);
    MSG_PRINTF(LOG_DBG, "OTI_ENVIRONMENT_ADDR  : %s\n", local_config_get_data("OTI_ENVIRONMENT_ADDR"));
    MSG_PRINTF(LOG_DBG, "EMQ_SERVER_ADDR       : %s\n", local_config_get_data("EMQ_SERVER_ADDR"));
    MSG_PRINTF(LOG_DBG, "PROXY_SERVER_ADDR     : %s\n", local_config_get_data("PROXY_SERVER_ADDR"));    
    MSG_PRINTF(LOG_DBG, "MBN_CONFIGURATION     : %s\n", local_config_get_data("MBN_CONFIGURATION"));
    MSG_PRINTF(LOG_DBG, "INIT_PROFILE_TYPE     : %s\n", local_config_get_data("INIT_PROFILE_TYPE"));
    MSG_PRINTF(LOG_DBG, "RPLMN_ENABLE          : %s\n", local_config_get_data("RPLMN_ENABLE"));
    MSG_PRINTF(LOG_DBG, "LOG_FILE_SIZE         : %s MB\n", local_config_get_data("LOG_FILE_SIZE"));
    MSG_PRINTF(LOG_DBG, "UICC_MODE             : %s\n", !rt_os_strcmp(local_config_get_data("UICC_MODE"), UICC_MODE_vUICC) ? "vUICC" : "eUICC");
    MSG_PRINTF(LOG_DBG, "MONITOR_LOG_LEVEL     : %s\n", local_config_get_data("MONITOR_LOG_LEVEL"));
    MSG_PRINTF(LOG_DBG, "AGENT_LOG_LEVEL       : %s\n", local_config_get_data("AGENT_LOG_LEVEL"));
}

int32_t init_config(void *arg)
{
    public_value_list_t *public_value_list = (public_value_list_t *)arg;
    int32_t pair_num = ARRAY_SIZE(g_config_items);
    config_item_t *items = g_config_items;
    config_info_t *infos = &g_config_info;

    config_init_file(CONFIG_FILE_PATH, pair_num, items);

    config_sync_global_info(infos, pair_num, items);
    
    ((public_value_list_t *)arg)->config_info = infos;
    log_set_param(LOG_PRINTF_FILE, infos->agent_log_level, infos->log_max_size);
        
    config_debug_cur_param(pair_num, items);

    return RT_SUCCESS;
}

/* remote config error code list */
typedef enum CONFIG_RESULT {
    CONFIG_NO_FAILURE           = 0,
    CONFIG_PART_OK_ERROR        = -1,       // part config items config ok, redtealink required
    CONFIG_MSG_PARSE_ERROR      = -3001,
    CONFIG_ALL_NONE_ERROR       = -3002,    // total config items all unsupported    
    CONFIG_ALL_FAIL_ERROR       = -3004,    // total config items all config fail    
    CONFIG_OTHER_ERROR          = -3099,
} config_result_e;

typedef struct CONFIG_ONLINE_PARAM {
    char                        tranId[64];
    config_result_e             result;
    int32_t                     count;  // total config item count
    char                        OTI_ENVIRONMENT_ADDR[32]; 
    char                        EMQ_SERVER_ADDR[32];      
    char                        PROXY_SERVER_ADDR[64];    
    int32_t                     MBN_CONFIGURATION;    
    int32_t                     INIT_PROFILE_TYPE;    
    int32_t                     RPLMN_ENABLE;         
    int32_t                     LOG_FILE_SIZE;        
    int32_t                     UICC_MODE;          
    char                        MONITOR_LOG_LEVEL[16];   
    char                        AGENT_LOG_LEVEL[16];      
    int32_t                     USAGE_ENABLE;         
    int32_t                     USAGE_FREQ;           
} config_online_param_t;

#if 0 // @ref cJSON.h
/* cJSON Types: */
#define cJSON_False             0
#define cJSON_True              1
#define cJSON_NULL              2
#define cJSON_Number            3
#define cJSON_String            4
#define cJSON_Array             5
#define cJSON_Object            6
#endif

#define CJSON_INT_TYPE          3

#define CONFIG_GET_ITEM_INT_DATA(json, param, tmp, item)\
    do {\
        tmp = cJSON_GetObjectItem(json, #item);\
        if (tmp) {\
            param->count++;\
            if (tmp->type == CJSON_INT_TYPE) {\
                param->item = tmp->valueint;\
                MSG_PRINTF(LOG_INFO, "%-20s : %d\n", #item, param->item);\
            } else {\
                param->item = 0xFFFFFFFF;\
                MSG_PRINTF(LOG_INFO, "%-20s : %d\n", #item, param->item);\
            }\
        } else {\
            param->item = 0xFFFFFFFF;\
            MSG_PRINTF(LOG_INFO, "%-20s : %d\n", #item, param->item);\
        }\
    } while(0)

#define CONFIG_GET_ITEM_STR_DATA(json, param, tmp, item)\
    do {\
        tmp = cJSON_GetObjectItem(json, #item);\
        if (tmp) {\
            param->count++;\
            snprintf((param->item), sizeof(param->item), "%s", tmp->valuestring);\
            MSG_PRINTF(LOG_INFO, "%-20s : %s\n", #item, param->item);\
        }\
    } while(0)

#define CONFIG_PACK_ITEM_STR_DATA(pair_num, items, json, item)\
    do {\
        char *item = config_get_data(#item, pair_num, items);\
        CJSON_ADD_NEW_STR_OBJ(json, item);\
    } while(0)

#define CONFIG_PACK_ITEM_INT_DATA(pair_num, items, json, item)\
    do {\
        char *tmp_str = config_get_data(#item, pair_num, items);\
        int32_t item = atoi(tmp_str);\
        CJSON_ADD_NEW_INT_OBJ(json, item);\
    } while(0)

#define cJSON_GET_JSON_DATA(json, item)\
    do {\
        item = cJSON_GetObjectItem(json, #item);\
    } while(0)

#define cJSON_PRINT_JSON_STR_DATA(json, item_str_out)\
    do {\
        item_str_out = cJSON_PrintUnformatted(json);\
    } while(0)

#define cJSON_DEBUG_JSON_STR_DATA(json)\
    do {\
        const char *tmp_str_out = cJSON_PrintUnformatted(json);\
        if (tmp_str_out) {\
            MSG_PRINTF(LOG_INFO, #json": %s\r\n", tmp_str_out);\
            cJSON_free((void *)tmp_str_out);\
        }\
    } while(0)

#define CONFIG_CHK_PINTER_NULL(p, ret_value)\
    do {\
        if (!p) {\
            MSG_PRINTF(LOG_WARN, #p" error\n");\
            ret = ret_value;\
            goto exit_entry;\
        }\
    } while(0)

static int32_t config_parser(const void *in, char *tran_id, void **out)
{
    int32_t ret = RT_ERROR;
    cJSON *msg = NULL;
    cJSON *tranId = NULL;
    cJSON *payload = NULL;
    cJSON *payload_json = NULL;
    cJSON *content = NULL;
    cJSON *config = NULL;
    cJSON *config_json = NULL;
    char *config_str = NULL;
    cJSON *policy = NULL;
    cJSON *tmp = NULL;
    config_online_param_t *param = NULL;
    static int8_t md5_out_pro[MD5_STRING_LENGTH + 1];
    int8_t md5_out_now[MD5_STRING_LENGTH + 1];

    get_md5_string((int8_t *)in, md5_out_now);
    md5_out_now[MD5_STRING_LENGTH] = '\0';
    if (rt_os_strcmp(md5_out_pro, md5_out_now) == 0) {
        MSG_PRINTF(LOG_ERR, "The data are the same!!\n");
        return ret;
    }
    rt_os_strcpy(md5_out_pro, md5_out_now);

    param = (config_online_param_t *)rt_os_malloc(sizeof(config_online_param_t));
    CONFIG_CHK_PINTER_NULL(param, -1);
    rt_os_memset(param, 0, sizeof(config_online_param_t));

    msg =  cJSON_Parse((const char *)in);
    CONFIG_CHK_PINTER_NULL(msg, -2);

    cJSON_GET_JSON_DATA(msg, tranId);
    CONFIG_CHK_PINTER_NULL(tranId, -3);
    
    rt_os_strcpy(tran_id, tranId->valuestring);
    rt_os_strncpy(param->tranId, tranId->valuestring, rt_os_strlen(tranId->valuestring));

    cJSON_GET_JSON_DATA(msg, payload);
    CONFIG_CHK_PINTER_NULL(param, -4);

    payload_json = cJSON_Parse((const char *)payload->valuestring);
    CONFIG_CHK_PINTER_NULL(payload_json, -5);
    cJSON_DEBUG_JSON_STR_DATA(payload_json);

    cJSON_GET_JSON_DATA(payload_json, content);
    CONFIG_CHK_PINTER_NULL(content, -6);

    cJSON_GET_JSON_DATA(content, config);
    CONFIG_CHK_PINTER_NULL(config, -7);
    cJSON_DEBUG_JSON_STR_DATA(config);

    config_json = cJSON_Parse((const char *)config->valuestring);
    CONFIG_CHK_PINTER_NULL(config_json, -8);
    cJSON_DEBUG_JSON_STR_DATA(config_json);

    /* get remote config item data */
    CONFIG_GET_ITEM_STR_DATA(config_json, param, tmp, OTI_ENVIRONMENT_ADDR);
    CONFIG_GET_ITEM_STR_DATA(config_json, param, tmp, EMQ_SERVER_ADDR);   
    CONFIG_GET_ITEM_STR_DATA(config_json, param, tmp, PROXY_SERVER_ADDR); 
    CONFIG_GET_ITEM_INT_DATA(config_json, param, tmp, MBN_CONFIGURATION);  
    CONFIG_GET_ITEM_INT_DATA(config_json, param, tmp, INIT_PROFILE_TYPE);  
    CONFIG_GET_ITEM_INT_DATA(config_json, param, tmp, RPLMN_ENABLE);       
    CONFIG_GET_ITEM_INT_DATA(config_json, param, tmp, LOG_FILE_SIZE);     
    CONFIG_GET_ITEM_INT_DATA(config_json, param, tmp, UICC_MODE);         
    CONFIG_GET_ITEM_STR_DATA(config_json, param, tmp, MONITOR_LOG_LEVEL);  
    CONFIG_GET_ITEM_STR_DATA(config_json, param, tmp, AGENT_LOG_LEVEL);   
    CONFIG_GET_ITEM_INT_DATA(config_json, param, tmp, USAGE_ENABLE);       
    CONFIG_GET_ITEM_INT_DATA(config_json, param, tmp, USAGE_FREQ);         

    *out = param;
    ret = RT_SUCCESS;
    
exit_entry:

    if (msg) {
        cJSON_Delete(msg);
        msg = NULL;
    }

    if (payload_json) {
        cJSON_Delete(payload_json);
        payload_json = NULL;
    }

    if (ret) {
        /* msg parse error, output error code */
        param->result = CONFIG_MSG_PARSE_ERROR;
        *out = param;
    }

    return ret;
}

static int32_t config_handle_items(const char *key, int32_t pair_num, config_item_t *items, const void *in)
{
    char *value;
    int32_t i = 0;
    int32_t ret = RT_ERROR;
    config_func config;
    
    for (i = 0; i < pair_num; i++) {
        if (!rt_os_strcmp(items[i].key, key)) {
            config = items[i].config;
            if (config) {
                ret = config(in, items[i].value);
            }
        }
    }

    return ret;
}

#define CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok, item)\
    do {\
        int32_t __l_ret = config_handle_items(#item, pair_num, items, (const void *)param->item);\
        if (!__l_ret) {\
            (ok)++;\
        }\
    } while(0)

static int32_t config_handler(const void *in, const char *event, void **out)
{
    int32_t ret = CONFIG_OTHER_ERROR;
    const config_online_param_t *param = (const config_online_param_t *)in;

    (void)out;
    if (param) {
        int32_t pair_num = ARRAY_SIZE(g_config_items);
        config_item_t *items = g_config_items;
        int32_t ok_cnt = 0;
        int32_t total_cnt = param->count;
    
        if (param->result != CONFIG_NO_FAILURE) {
            ret = param->result;
            goto exit_entry;
        }

        if (total_cnt == 0) {
            ret = CONFIG_ALL_NONE_ERROR;
            goto exit_entry;
        }

        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, OTI_ENVIRONMENT_ADDR);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, EMQ_SERVER_ADDR);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, PROXY_SERVER_ADDR);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, MBN_CONFIGURATION);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, INIT_PROFILE_TYPE);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, RPLMN_ENABLE);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, LOG_FILE_SIZE);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, UICC_MODE);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, MONITOR_LOG_LEVEL);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, AGENT_LOG_LEVEL);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, USAGE_ENABLE);
        CONFIG_HANDLE_ITEM_DATA(pair_num, items, param, ok_cnt, USAGE_FREQ);

        MSG_PRINTF(LOG_INFO, "total: %d, ok: %d\r\n", total_cnt, ok_cnt);

        if (ok_cnt == total_cnt) {
            ret = CONFIG_NO_FAILURE;      // all ok      
        } else if (ok_cnt == 0) {
            ret = CONFIG_ALL_FAIL_ERROR;  // all fail
        } else {
            ret = CONFIG_PART_OK_ERROR;   // part ok
        }

        if (ok_cnt > 0) {
            /* update config file */
            config_write_file(CONFIG_FILE_PATH, pair_num, items);
        }
    }

exit_entry:

    /* release input param memory */
    if (param) {
        rt_os_free((void *)param);
        param = NULL;
    }

    return ret;
}

static cJSON *config_packer(void *arg)
{
    int32_t ret = 0;
    cJSON *on_config = NULL;
    char *config = NULL;
    cJSON *config_json = NULL;
    int32_t pair_num = ARRAY_SIZE(g_config_items);
    config_item_t *items = g_config_items;       
    
    on_config = cJSON_CreateObject();
    if (!on_config) {
        MSG_PRINTF(LOG_WARN, "The on_config is error\n");
        ret = -1;
        goto exit_entry;
    }

    config_json = cJSON_CreateObject();
    if (!config_json) {
        MSG_PRINTF(LOG_WARN, "The config_json is error\n");
        ret = -2;
        goto exit_entry;
    }

    /* always upload current config item list */
    CONFIG_PACK_ITEM_STR_DATA(pair_num, items, config_json, OTI_ENVIRONMENT_ADDR);
    CONFIG_PACK_ITEM_STR_DATA(pair_num, items, config_json, EMQ_SERVER_ADDR);
    CONFIG_PACK_ITEM_STR_DATA(pair_num, items, config_json, PROXY_SERVER_ADDR);
    CONFIG_PACK_ITEM_INT_DATA(pair_num, items, config_json, MBN_CONFIGURATION);
    CONFIG_PACK_ITEM_INT_DATA(pair_num, items, config_json, INIT_PROFILE_TYPE);
    CONFIG_PACK_ITEM_INT_DATA(pair_num, items, config_json, RPLMN_ENABLE);
    CONFIG_PACK_ITEM_INT_DATA(pair_num, items, config_json, LOG_FILE_SIZE);
    CONFIG_PACK_ITEM_INT_DATA(pair_num, items, config_json, UICC_MODE);
    CONFIG_PACK_ITEM_STR_DATA(pair_num, items, config_json, MONITOR_LOG_LEVEL);
    CONFIG_PACK_ITEM_STR_DATA(pair_num, items, config_json, AGENT_LOG_LEVEL);
    CONFIG_PACK_ITEM_INT_DATA(pair_num, items, config_json, USAGE_ENABLE);
    CONFIG_PACK_ITEM_INT_DATA(pair_num, items, config_json, USAGE_FREQ);

    cJSON_PRINT_JSON_STR_DATA(config_json, config);
    CJSON_ADD_NEW_STR_OBJ(on_config, config);

    ret = 0;
    
exit_entry:

    if (config_json) {
        cJSON_Delete(config_json);
        config_json = NULL;
    }

    if (config) {
        cJSON_free(config);
        config = NULL;
    }

    return !ret ? on_config : NULL;   
}

DOWNSTREAM_METHOD_OBJ_INIT(CONFIG, MSG_ID_IDLE, ON_CONFIG, config_parser, config_handler);

UPLOAD_EVENT_OBJ_INIT(ON_CONFIG, TOPIC_DEVICEID_OR_EID, config_packer);

