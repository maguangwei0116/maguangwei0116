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

#define M_BYTES                             (1024 * 1024)
#define MAX_LINE_SIZE                       128
#define MAX_VALUE_SIZE                      30
#define LINK_SYMBOL                         '='  // key - value pair 之间的连接符
#define ANNOTATION_SYMBOL                   '#'  // 注释标识符
#define CONFIG_FILE_PATH                    "/data/redtea/rt_config.ini"
#define IS_SPACES(x)                        ( ' ' == (x) || '\t' == (x) || '\n' == (x) || '\r' == (x) || '\f' == (x) || '\b' == (x) )  // 判定是否为空白符

#define UICC_MODE_vUICC                     "0"
#define UICC_MODE_eUICC                     "1"

#if (CFG_ENV_TYPE_PROD)
#define DEFAULT_OTI_ENVIRONMENT_ADDR        "52.220.34.227"         // 默认生产环境
#define DEFAULT_EMQ_SERVER_ADDR             "18.136.190.97"         // 默认生产环境EMQ地址
#define DEFAULT_PROXY_SERVER_ADDR           "smdp.redtea.io"        //默认生产环境smdp地址
#elif (CFG_ENV_TYPE_STAGING)
#define DEFAULT_OTI_ENVIRONMENT_ADDR        "54.222.248.186"        // 默认staging环境
#define DEFAULT_EMQ_SERVER_ADDR             "13.229.31.234"         // 默认staging环境EMQ地址
#define DEFAULT_PROXY_SERVER_ADDR           "smdp-test.redtea.io"   //默认staging环境smdp地址
#endif

#define DEFAULT_MBN_CONFIGURATION           "1"                     // 默认开启MBN配置功能 
#define DEFAULT_LOG_FILE_SIZE               "1"                     // 默认log大小为1M   
#define DEFAULT_INIT_PROFILE_TYPE           "2"                     // 默认启用上一张卡
#define DEFAULT_RPLMN_ENABLE                "1"                     // 默认开启rplmn配置功能
#define DEFAULT_UICC_MODE                   UICC_MODE_eUICC         // 默认使用QMI通道，即实体卡模式(eUICC)          
#define DEFAULT_MONITOR_LOG_LEVEL           "4"                     // LOG_INFO
#define DEFAULT_AGENT_LOG_LEVEL             "4"                     // LOG_INFO

char *OTI_ENVIRONMENT_ADDR;
char *EMQ_SERVER_ADDR;    
char *PROXY_SERVER_ADDR;   
char *MBN_CONFIGURATION;   
char *LOG_FILE_SIZE;       
char *INIT_PROFILE_TYPE;   
char *RPLMN_ENABLE;        
char *UICC_MODE;           
char *MONITOR_LOG_LEVEL;   
char *AGENT_LOG_LEVEL;

typedef enum CONFIG_DATA_TYPE {
    CONFIG_STRING           = 0,
    CONFIG_INTEGER          = 1, 
} config_data_type_e;

typedef struct CONFIG_ITEM {    
    const char *            key;
    char **                 value;
    char *                  def_value;
    config_data_type_e      type;
    const char *            annotation;
} config_item_t;

#define ITEM(value, type, annotation)\
{\
    #value, &value, DEFAULT_##value, type, annotation,\
}

static config_item_t g_config_items[] = 
{
    ITEM(OTI_ENVIRONMENT_ADDR,    CONFIG_STRING,  "OTI server addr: stage(54.222.248.186) or prod(52.220.34.227)"), 
    ITEM(EMQ_SERVER_ADDR,         CONFIG_STRING,  "EMQ server addr stage(13.229.31.234) prod(18.136.190.97)"),
    ITEM(PROXY_SERVER_ADDR,       CONFIG_STRING,  "SMDP server addr: stage(smdp-test.redtea.io) prod(smdp.redtea.io) qa(smdp-test.redtea.io)"),
    ITEM(MBN_CONFIGURATION,       CONFIG_INTEGER, "Whether the config MBN"),
    ITEM(LOG_FILE_SIZE,           CONFIG_INTEGER, "The max size of rt_log file (M)"),
    ITEM(INIT_PROFILE_TYPE,       CONFIG_INTEGER, "The rules of the first boot option profile (0:Provisioning 1:Operational 2:last)"),
    ITEM(RPLMN_ENABLE,            CONFIG_INTEGER, "Whether set the rplmn"),
    ITEM(UICC_MODE,               CONFIG_INTEGER, "The mode of UICC (0:vUICC  1:eUICC)"),
};

#define g_config_items_value(i)     (*g_config_items[i].value)
#define g_config_items_def_value(i) (g_config_items[i].def_value)

static uint32_t msg_string_to_int(const char *str)
{
    uint32_t length = 0;
    
    if (!str) {
        MSG_PRINTF(LOG_WARN, "The string is error\n");
        return 0;
    }
    
    while (*str != '\0') {
        if ((*str >= '0') && (*str <= '9')) {
            length = length * 10 + *str - '0';
        }
        str++;
    }
    
    return length;
}

static int32_t config_set_data(const char *key, const char *value)
{
    int32_t len;
    int32_t i = 0;
    
    for (i = 0; i < ARRAY_SIZE(g_config_items); i++) {
        if (!rt_os_strcmp(g_config_items[i].key, key)) {
            len = rt_os_strlen(value);
            rt_os_memcpy(g_config_items_value(i), value, len);
            g_config_items_value(i)[len] = '\0';
        }
    }
    
    return RT_SUCCESS;
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
            ANNOTATION_SYMBOL, g_config_items[i].annotation, g_config_items[i].key, LINK_SYMBOL, g_config_items_value(i));
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
    while (*temp == ' ' || *temp == '\t') {
        ++temp;
    }

    start = temp;
    temp = str_in + rt_os_strlen(str_in) - 1;

    while (*temp == ' ' || *temp == '\t') {
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

    while (*p1 != ' ' && *p1 != '\r' && *p1 != '\n') {
        p1++;
    }
    rt_os_memcpy(key, p0, p1-p0);
    
    p++;
    p0 = p;
    while (!(*p0 != ' ' && *p0 != '\r' && *p0 != '\n')) {
        p0++;
    }

    p1 = p0 + 1;
    while (*p1 != ' ' && *p1 != '\r' && *p1 != '\n') {
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
            if (line_data_out[0] == ANNOTATION_SYMBOL || line_data_out[0] == '\r' || line_data_out[0] == '\n') {
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
        rt_os_strcpy(g_config_items_value(i), g_config_items_def_value(i));
    }

    /* write defconfig file */
    config_write_file(CONFIG_FILE_PATH, ARRAY_SIZE(g_config_items));
    MSG_PRINTF(LOG_DBG, "Create default param for [%s] environment !!!\r\n", CFG_ENV_TYPE);
}

static void config_debug_cur_param(void)
{
    MSG_PRINTF(LOG_DBG, "OTI_ENVIRONMENT_ADDR  : %s\n", OTI_ENVIRONMENT_ADDR);
    MSG_PRINTF(LOG_DBG, "EMQ_SERVER_ADDR       : %s\n", EMQ_SERVER_ADDR);
    MSG_PRINTF(LOG_DBG, "PROXY_SERVER_ADDR     : %s\n", PROXY_SERVER_ADDR);
    MSG_PRINTF(LOG_DBG, "LOG_FILE_SIZE         : %s MB\n", LOG_FILE_SIZE);
    MSG_PRINTF(LOG_DBG, "MBN_CONFIGURATION     : %s\n", MBN_CONFIGURATION);
    MSG_PRINTF(LOG_DBG, "INIT_PROFILE_TYPE     : %s\n", INIT_PROFILE_TYPE);
    MSG_PRINTF(LOG_DBG, "RPLMN_ENABLE          : %s\n", RPLMN_ENABLE);
    MSG_PRINTF(LOG_DBG, "UICC_MODE             : %s\n", !rt_os_strcmp(UICC_MODE, UICC_MODE_vUICC) ? "vUICC" : "eUICC");   
}

static int32_t config_memory_init(void)
{
    int32_t i;

    for(i=0; i < ARRAY_SIZE(g_config_items); i++) {
        *g_config_items[i].value = (char *)rt_os_malloc(MAX_VALUE_SIZE * sizeof(char));
        if (!(*g_config_items[i].value)) {
            MSG_PRINTF(LOG_WARN, "config memory fail !\r\n");
            return RT_ERROR;
        }
        (*g_config_items[i].value)[0] = '\0';
    } 

    return RT_SUCCESS;
}

int32_t init_config(void *arg)
{
    public_value_list_t *public_value_list = (public_value_list_t *)arg;

    config_memory_init();

    if (0 || rt_os_access(CONFIG_FILE_PATH, F_OK) == RT_ERROR) {
        config_create_default_file();
    } 

    config_parse_file();

    config_debug_cur_param();

    public_value_list->lpa_channel_type = !rt_os_strcmp(UICC_MODE, UICC_MODE_vUICC) ? LPA_CHANNEL_BY_IPC : LPA_CHANNEL_BY_QMI;
    MSG_PRINTF(LOG_DBG, "public_value_list->lpa_channel_type=%d\r\n", public_value_list->lpa_channel_type);

    public_value_list->log_max_size = msg_string_to_int((const char *)LOG_FILE_SIZE) * M_BYTES;
    MSG_PRINTF(LOG_DBG, "public_value_list->log_max_size=%d bytes\r\n", public_value_list->log_max_size);

    return RT_SUCCESS;
}

