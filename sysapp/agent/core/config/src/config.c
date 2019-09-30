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
ITEM(MBN_CONFIGURATION,     "1",                    "Whether the config MBN"),
ITEM(INIT_PROFILE_TYPE,     "2",                    "The rules of the first boot option profile (0:Provisioning 1:Operational 2:last)"),
ITEM(RPLMN_ENABLE,          "1",                    "Whether set the rplmn"),
ITEM(LOG_FILE_SIZE,         "1",                    "The max size of rt_log file (MB)"),
ITEM(UICC_MODE,             "1",                    "The mode of UICC (0:vUICC  1:eUICC)"),
ITEM(MONITOR_LOG_LEVEL,     "LOG_INFO",             "The log level of monitor (LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO)"),
ITEM(AGENT_LOG_LEVEL,       "LOG_INFO",             "The log level of agent (LOG_NONE LOG_ERR LOG_WARN LOG_DBG LOG_INFO)"),
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

static int32_t config_set_data(const char *key, const char *value)
{
    int32_t i = 0;
    
    for (i = 0; i < ARRAY_SIZE(g_config_items); i++) {
        if (!rt_os_strcmp(g_config_items[i].key, key)) {
            snprintf(g_config_items[i].value, sizeof(g_config_items[i].value), "%s", value);
        }
    }
    
    return RT_SUCCESS;
}

static char *config_get_data(const char *key)
{
    int32_t i = 0;
    
    for (i = 0; i < ARRAY_SIZE(g_config_items); i++) {
        if (!rt_os_strcmp(g_config_items[i].key, key)) {
            return g_config_items[i].value;
        }
    }
    
    return "";
}

/* save config file */
static int32_t config_write_file(int8_t *file_path, int32_t pair_num)
{
    int32_t i = 0;
    rt_fshandle_t fp;
    char item_data[MAX_LINE_SIZE];

    fp = linux_fopen(file_path, "w");
    if (!fp) {
        return RT_ERROR;
    }

    for (; i < pair_num; i++) {
        snprintf(item_data, sizeof(item_data), "%c%s\n%s %c %s\n\n", 
            ANNOTATION_SYMBOL, g_config_items[i].annotation, g_config_items[i].key, LINK_SYMBOL, g_config_items[i].value);
        linux_fwrite(item_data, 1, rt_os_strlen(item_data), fp);
    }
    
    linux_fclose(fp);
    fp = NULL;
    
    return RT_SUCCESS;
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

static int32_t config_read_file(const char *file_path, int32_t pair_num)
{
    int32_t ret;
    rt_fshandle_t fp = NULL;
    int32_t i = 0;    
    char line_data[MAX_LINE_SIZE];
    char line_data_out[MAX_LINE_SIZE];
    char key[MAX_VALUE_SIZE];
    char value[MAX_VALUE_SIZE];

    fp = linux_fopen(file_path, "r");
    if(!fp) {
        MSG_PRINTF(LOG_WARN, "can't open file %s\n", file_path);
        ret = RT_ERROR;
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
                config_set_data((const char *)key, (const char *)value);
            }
        }
    }
    ret = RT_SUCCESS;

exit_entry:
    if (fp) {
        linux_fclose(fp);
    }

    return ret;
}      

/**
* parse config file
*/
static void config_parse_file(void)
{
    config_read_file(CONFIG_FILE_PATH, ARRAY_SIZE(g_config_items));
}

/**
* update config file
*/
static void config_create_default_file(void)
{
    int32_t i = 0;

    /* load default config value */
    for (i = 0; i < ARRAY_SIZE(g_config_items); i++) {
        snprintf(g_config_items[i].value, sizeof(g_config_items[i].value), "%s", g_config_items[i].def_value);
    }

    /* write defconfig file */
    config_write_file(CONFIG_FILE_PATH, ARRAY_SIZE(g_config_items));
    MSG_PRINTF(LOG_DBG, "Create default param for [%s] environment !!!\r\n", CFG_ENV_TYPE);
}

static void config_debug_cur_param(void)
{
    MSG_PRINTF(LOG_DBG, "OTI_ENVIRONMENT_ADDR  : %s\n", g_config_info.oti_addr);
    MSG_PRINTF(LOG_DBG, "EMQ_SERVER_ADDR       : %s\n", g_config_info.emq_addr);
    MSG_PRINTF(LOG_DBG, "PROXY_SERVER_ADDR     : %s\n", g_config_info.proxy_addr);    
    MSG_PRINTF(LOG_DBG, "MBN_CONFIGURATION     : %s\n", config_get_data("MBN_CONFIGURATION"));
    MSG_PRINTF(LOG_DBG, "INIT_PROFILE_TYPE     : %s\n", config_get_data("INIT_PROFILE_TYPE"));
    MSG_PRINTF(LOG_DBG, "RPLMN_ENABLE          : %s\n", config_get_data("RPLMN_ENABLE"));
    MSG_PRINTF(LOG_DBG, "LOG_FILE_SIZE         : %s MB\n", config_get_data("LOG_FILE_SIZE"));
    MSG_PRINTF(LOG_DBG, "UICC_MODE             : %s\n", !rt_os_strcmp(config_get_data("UICC_MODE"), UICC_MODE_vUICC) ? "vUICC" : "eUICC");
    MSG_PRINTF(LOG_DBG, "MONITOR_LOG_LEVEL     : %s\n", config_get_data("MONITOR_LOG_LEVEL"));
    MSG_PRINTF(LOG_DBG, "AGENT_LOG_LEVEL       : %s\n", config_get_data("AGENT_LOG_LEVEL"));
}

int32_t init_config(void *arg)
{
    public_value_list_t *public_value_list = (public_value_list_t *)arg;

    if (rt_os_access(CONFIG_FILE_PATH, F_OK) == RT_ERROR) {
        config_create_default_file();
    }

    config_parse_file();

    g_config_info.oti_addr = config_get_data("OTI_ENVIRONMENT_ADDR");
    g_config_info.emq_addr = config_get_data("EMQ_SERVER_ADDR");
    g_config_info.proxy_addr = config_get_data("PROXY_SERVER_ADDR");
    g_config_info.lpa_channel_type = !rt_os_strcmp(config_get_data("UICC_MODE"), UICC_MODE_vUICC) ? \
                                                    LPA_CHANNEL_BY_IPC : LPA_CHANNEL_BY_QMI;
    g_config_info.log_max_size = msg_string_to_int(config_get_data("LOG_FILE_SIZE")) * M_BYTES;
    g_config_info.monitor_log_level = log_get_level(config_get_data("MONITOR_LOG_LEVEL"));
    g_config_info.agent_log_level = log_get_level(config_get_data("AGENT_LOG_LEVEL"));
    
    ((public_value_list_t *)arg)->config_info = &g_config_info;

    config_debug_cur_param();

    return RT_SUCCESS;
}

