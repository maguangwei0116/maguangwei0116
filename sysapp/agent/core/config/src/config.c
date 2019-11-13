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

typedef struct CONFIG_ITEM {    
    const char *            key;
    char                    value[MAX_VALUE_SIZE];
    char *                  def_value;
    const char *            annotation;
} config_item_t;

#define ITEM(value, def_value, annotation)\
{\
    #value, {0}, def_value, annotation,\
}

static config_item_t g_config_items[] = 
{
/*     item_name            default_value           annotation          */
#if (CFG_ENV_TYPE_PROD)
ITEM(OTI_ENVIRONMENT_ADDR,  "52.220.34.227",        "OTI server addr: stage(54.222.248.186) or prod(52.220.34.227)"), 
ITEM(EMQ_SERVER_ADDR,       "18.136.190.97",        "EMQ server addr: stage(13.229.31.234) prod(18.136.190.97)"),
ITEM(PROXY_SERVER_ADDR,     "smdp.redtea.io",       "SMDP server addr: stage(smdp-test.redtea.io) prod(smdp.redtea.io) qa(smdp-test.redtea.io)"),
#else
ITEM(OTI_ENVIRONMENT_ADDR,  "54.222.248.186",       "OTI server addr: stage(54.222.248.186) or prod(52.220.34.227)"), 
ITEM(EMQ_SERVER_ADDR,       "13.229.31.234",        "EMQ server addr: stage(13.229.31.234) prod(18.136.190.97)"),
ITEM(PROXY_SERVER_ADDR,     "smdp-test.redtea.io",  "SMDP server addr: stage(smdp-test.redtea.io) prod(smdp.redtea.io) qa(smdp-test.redtea.io)"),
#endif
ITEM(MBN_CONFIGURATION,     "1",                    "Whether config MBN (0:disable  1:enable)"),
ITEM(INIT_PROFILE_TYPE,     "2",                    "The rules of the first boot option profile (0:Provisioning 1:Operational 2:last)"),
ITEM(RPLMN_ENABLE,          "1",                    "Whether set rplmn (0:disable  1:enable)"),
ITEM(LOG_FILE_SIZE,         "1",                    "The max size of rt_log file (MB)"),
ITEM(UICC_MODE,             "1",                    "The mode of UICC (0:vUICC  1:eUICC)"),
#if (CFG_SOFTWARE_TYPE_RELEASE)
ITEM(MONITOR_LOG_LEVEL,     "LOG_WARN",             "The log level of monitor (LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO)"),
ITEM(AGENT_LOG_LEVEL,       "LOG_WARN",             "The log level of agent (LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO)"),
#else
ITEM(MONITOR_LOG_LEVEL,     "LOG_INFO",             "The log level of monitor (LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO)"),
ITEM(AGENT_LOG_LEVEL,       "LOG_INFO",             "The log level of agent (LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO)"),
#endif
ITEM(USAGE_ENABLE,          "0",                    "Whether enable upload user traffic (0:disable  1:enable)"),
ITEM(USAGE_FREQ,            "60",                   "Frequency of upload user traffic ( 60 <= x <= 1440 Mins)"),
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

static int32_t config_init_global_info(config_info_t *infos, int32_t pair_num, const config_item_t *items)
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

    config_init_global_info(infos, pair_num, items);
    
    ((public_value_list_t *)arg)->config_info = infos;
    log_set_param(LOG_PRINTF_FILE, infos->agent_log_level, infos->log_max_size);
        
    config_debug_cur_param(pair_num, items);

    return RT_SUCCESS;
}

static int32_t config_parser(const void *in, char *tran_id, void **out)
{
    int32_t ret;
    cJSON *msg = NULL;
    cJSON *tranId = NULL;
    cJSON *payload = NULL;
    char *payload_str = NULL;
    cJSON *payload_json = NULL;
    cJSON *content = NULL;
    cJSON *target = NULL;
    cJSON *policy = NULL;
    cJSON *tmp = NULL;
    //ota_upgrade_param_t *param = NULL;
    static int8_t md5_out_pro[MD5_STRING_LENGTH + 1];
    int8_t md5_out_now[MD5_STRING_LENGTH + 1];

    get_md5_string((int8_t *)in, md5_out_now);
    md5_out_now[MD5_STRING_LENGTH] = '\0';
    if (rt_os_strcmp(md5_out_pro, md5_out_now) == 0) {
        MSG_PRINTF(LOG_ERR, "The data are the same!!\n");
        return ret;
    }
    rt_os_strcpy(md5_out_pro, md5_out_now);
/*
    msg =  cJSON_Parse((const char *)in);
    OTA_CHK_PINTER_NULL(msg, -1);

    cJSON_GET_JSON_DATA(msg, tranId);
    OTA_CHK_PINTER_NULL(tranId, -2);
    
    rt_os_strcpy(tran_id, tranId->valuestring);

    param = (ota_upgrade_param_t *)rt_os_malloc(sizeof(ota_upgrade_param_t));
    OTA_CHK_PINTER_NULL(param, -3);
    rt_os_memset(param, 0, sizeof(ota_upgrade_param_t));
    rt_os_strncpy(param->tranId, tranId->valuestring, rt_os_strlen(tranId->valuestring));

    cJSON_GET_JSON_DATA(msg, payload);
    OTA_CHK_PINTER_NULL(param, -4);

    payload_json = cJSON_Parse((const char *)payload->valuestring);
    OTA_CHK_PINTER_NULL(payload_json, -5);
    cJSON_DEBUG_JSON_STR_DATA(payload_json);

    cJSON_GET_JSON_DATA(payload_json, content);
    OTA_CHK_PINTER_NULL(content, -6);

    cJSON_GET_JSON_DATA(content, target);
    OTA_CHK_PINTER_NULL(target, -7);
    cJSON_DEBUG_JSON_STR_DATA(target);
    cJSON_GET_STR_DATA(target, name, param->target.name, sizeof(param->target.name), tmp);
    cJSON_GET_STR_DATA(target, version, param->target.version, sizeof(param->target.version), tmp);
    cJSON_GET_STR_DATA(target, chipModel, param->target.chipModel, sizeof(param->target.chipModel), tmp);
    cJSON_GET_STR_DATA(target, ticket, param->target.ticket, sizeof(param->target.ticket), tmp);
    cJSON_GET_INT_DATA(target, type, param->target.type, tmp);
    cJSON_GET_INT_DATA(target, size, param->target.size, tmp);
    cJSON_GET_STR_DATA(target, fileHash, param->target.fileHash, sizeof(param->target.fileHash), tmp);

    cJSON_GET_JSON_DATA(content, policy);
    OTA_CHK_PINTER_NULL(policy, -8);
    cJSON_DEBUG_JSON_STR_DATA(policy);
    cJSON_GET_INT_DATA(policy, forced, param->policy.forced, tmp);
    cJSON_GET_STR_DATA(policy, executionType, param->policy.executionType, sizeof(param->policy.executionType), tmp);
    cJSON_GET_INT_DATA(policy, profileType, param->policy.profileType, tmp);
    cJSON_GET_INT_DATA(policy, retryAttempts, param->policy.retryAttempts, tmp);
    cJSON_GET_INT_DATA(policy, retryInterval, param->policy.retryInterval, tmp);

    *out = param;
    ret = RT_SUCCESS;
*/
exit_entry:

    if (msg) {
        cJSON_Delete(msg);
        msg = NULL;
    }

    if (payload_json) {
        cJSON_Delete(payload_json);
        payload_json = NULL;
    }
/*
    if (ret && param) {
        rt_os_free(param);
        param = NULL;
    }
*/
    return ret;
}

static int32_t config_handler(const void *in, const char *event, void **out)
{
    #if 0
    int32_t ret = -1;
    const ota_upgrade_param_t *param = (const ota_upgrade_param_t *)in;

    (void)out;
    if (param) {
        //ret = ota_upgrade_start(param, event, NULL);

        /* release input param memory */
        if (param) {
            rt_os_free((void *)param);
            param = NULL;
        }
    }

exit_entry:

    return ret;
    #endif
}

static cJSON *config_packer(void *arg)
{
    int32_t ret = 0;
    cJSON *app_version = NULL;
    const upgrade_struct_t *upgrade = (const upgrade_struct_t *)arg;
    const char *name = "";
    const char *version = "";
    const char *chipModel = "";
    int32_t type;

    if (upgrade) {
        name = upgrade->targetName;
        version = upgrade->targetVersion;
        chipModel = upgrade->targetChipModel;
        type = upgrade->type;
    } else {
        MSG_PRINTF(LOG_WARN, "error param input !\n");
    }

    app_version = cJSON_CreateObject();
    if (!app_version) {
        MSG_PRINTF(LOG_WARN, "The app_version is error\n");
        ret = -1;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(app_version, name);
    CJSON_ADD_NEW_STR_OBJ(app_version, version);
    CJSON_ADD_NEW_STR_OBJ(app_version, chipModel);
    CJSON_ADD_NEW_INT_OBJ(app_version, type);
    
    ret = 0;
    
exit_entry:

    return !ret ? app_version : NULL;   
}

DOWNSTREAM_METHOD_OBJ_INIT(CONFIG, MSG_ID_OTA_UPGRADE, ON_CONFIG, config_parser, config_handler);

UPLOAD_EVENT_OBJ_INIT(ON_CONFIG, TOPIC_DEVICEID_OR_EID, config_packer);

