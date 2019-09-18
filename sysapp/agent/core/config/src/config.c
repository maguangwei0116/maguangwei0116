/**
* �������ļ����ж�д����
* ���������� key - value pair ����ʽ�洢
* �������ӷ���ע�ͱ���� c �ļ��ж���
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include "rt_type.h"
#include "file.h"
#include "rt_os.h"
#include "agent_queue.h"

/************************************debug***********************************/

/************************************fallback***********************************/
#define DEFAULT_DIS_CONNECT_WAIT_TIME           100  // Ĭ��fallbackΪ5��
// #define DEFAULT_SEED_CARD_FIRST                 0  // Ĭ�ϲ������ӿ�����

/************************************general***********************************/
#if (CFG_ENV_TYPE_PROD)
#define DEFAULT_OTI_ENVIRONMENT_ADDR        "52.220.34.227"         // Ĭ����������
#define DEFAULT_EMQ_SERVER_ADDR             "18.136.190.97"         // Ĭ����������EMQ��ַ
#define DEFAULT_PROXY_SERVER_ADDR           "smdp.redtea.io"        //Ĭ����������smdp��ַ
#elif (CFG_ENV_TYPE_STAGING)
#define DEFAULT_OTI_ENVIRONMENT_ADDR        "54.222.248.186"        // Ĭ��staging����
#define DEFAULT_EMQ_SERVER_ADDR             "13.229.31.234"         // Ĭ��staging����EMQ��ַ
#define DEFAULT_PROXY_SERVER_ADDR           "smdp-test.redtea.io"   //Ĭ��staging����smdp��ַ
#endif

#define DEFAULT_CARD_TYPE_FLAG              1  // �Ƿ�����card_type�ļ�
#define DEFAULT_MBN_CONFIGURATION           1  // Ĭ�Ͽ���MBN���ù���
#define DEFAULT_LOG_FILE_SIZE               1  // Ĭ��log��СΪ1M
#define DEFAULT_MBN_CONFIGURATION           1  // Ĭ�Ͽ���MBN���ù���
#define DEFAULT_INIT_PROFILE_TYPE           2  // Ĭ��������һ�ſ�
#define DEFAULT_RPLMN_ENABLE                1  //Ĭ�Ͽ���rplmn���ù���
#define DEFAULT_UICC_MODE                   UICC_MODE_eUICC  //Ĭ��ʹ��QMIͨ������ʵ�忨ģʽ(eUICC)

/********************************platform**************************************/

#define MAX_VALUE_SIZE                      30
#define LINK_SYMBOL                         "="  // key - value pair ֮������ӷ�
#define ANNOTATION_SYMBOL                   "#"  // ע�ͱ�ʶ��
#define CONFIG_FILE_PATH                    "/data/redtea/rt_config.ini"
#define IS_SPACES(x)                        ( ' ' == (x) || '\t' == (x) || '\n' == (x) || '\r' == (x) || '\f' == (x) || '\b' == (x) )  // �ж��Ƿ�Ϊ�հ׷�

/* The keyname of config item*/
static const char *keys[] = {
        "DIS_CONNECT_WAIT_TIME",
        "OTI_ENVIRONMENT_ADDR",
        "EMQ_SERVER_ADDR",
        "PROXY_SERVER_ADDR",
        "MBN_CONFIGURATION",
        "LOG_FILE_SIZE",
        "INIT_PROFILE_TYPE",
        "RPLMN_ENABLE",
        "UICC_MODE",
};

/* The description of config item */
static const char *annotations[] = {
        "Broken network monitorning time",
        "The address of OTI server stage(54.222.248.186) or prod(52.220.34.227)",
        "The address of EMQ server stage(13.229.31.234) prod(18.136.190.97)",
        "The address of SMDP server stage(smdp-test.redtea.io) prod(smdp.redtea.io) qa(smdp-test.redtea.io)",
        "Whether the config MBN",
        "The max size of rt_log file (M)",
        "The rules of the first boot option profile (0:Provisioning 1:Operational 2:last)",
        "Whether set the rplmn",
        "The mode of UICC (0:vUICC  1:eUICC)"
};

/* the keyvalue of config item */
static int8_t *values[ARRAY_SIZE(keys)];

int8_t *OTI_ENVIRONMENT_ADDR = DEFAULT_OTI_ENVIRONMENT_ADDR;  // Ĭ�ϻ���Ϊprod
int8_t *EMQ_SERVER_ADDR = DEFAULT_EMQ_SERVER_ADDR;  // Ĭ��prod emq 
int8_t *PROXY_SERVER_ADDR = DEFAULT_PROXY_SERVER_ADDR;  // Ĭ��smdp address

int32_t DIS_CONNECT_WAIT_TIME = DEFAULT_DIS_CONNECT_WAIT_TIME;  // �������ʱ�䣬Ĭ��5����
int32_t MBN_CONFIGURATION = DEFAULT_MBN_CONFIGURATION;  // MBN���ÿ���
int32_t LOG_FILE_SIZE = DEFAULT_LOG_FILE_SIZE;  // Ĭ��log�ļ��Ĵ�С
int32_t INIT_PROFILE_TYPE = DEFAULT_INIT_PROFILE_TYPE;  // Ĭ��ʹ����һ�ſ�����
int32_t RPLMN_ENABLE = DEFAULT_RPLMN_ENABLE;  //rplmnĬ�ϴ�����
int32_t UICC_MODE = DEFAULT_UICC_MODE;  // Ĭ��ʹ��eUICC

/*****************************************************************************
 * FUNCTION
 *  msg_string_to_int
 * DESCRIPTION
 *  input string output int value.
 * PARAMETERS
 *  str
 * RETURNS
 *  int
 *****************************************************************************/
static uint32_t msg_string_to_int(uint8_t* str)
{
    uint32_t length = 0;
    if (str == NULL) {
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

/**
* д�����ļ�������ļ��Ѵ��ڻ����ԭ������
* ������ key - value pair ����ʽ�洢
* ��ʽΪ key=value
* �����Ҫ��ĳһ key - value pair ǰ���ע��
* ���� annotation_array ��Ӧ�������
* @params   file_path        �����ļ�·��
* @params   key_array        ��ż������飬�� key_array һһ��Ӧ
* @params   value_array      ���ֵ�����飬�� value_array һһ��Ӧ
* @params   annotation_array ���ע�͵����飬�� key - value pair һһ��Ӧ������ü�ֵ�Բ���Ҫע�ͣ����Ӧ������NULL
* @params   pair_num         key - value pair ������
* @return   �ɹ�����0�����򷵻�1
*/
static int32_t write_config_file(int8_t *file_path, const char **key_array, int8_t **value_array, const char **annotation_array, int32_t pair_num)
{
    int32_t i = 0;
    rt_fshandle_t fp;

    fp = rt_fopen(file_path, "w");
    if (NULL == fp) {
        return RT_ERROR;
    }

    for (; i < pair_num; i++) {
        if (NULL != key_array[i]) {
            if (NULL != annotation_array[i]) {
                rt_fwrite(ANNOTATION_SYMBOL, sizeof(int8_t), rt_os_strlen(ANNOTATION_SYMBOL), fp);
                rt_fwrite(annotation_array[i], sizeof(int8_t), rt_os_strlen((void *)annotation_array[i]), fp);
                rt_fwrite("\n", sizeof(int8_t), 1, fp);
            }
            rt_fwrite(key_array[i], sizeof(int8_t), rt_os_strlen((void *)key_array[i]), fp);
            rt_fwrite(" ", sizeof(int8_t), 1, fp);
            rt_fwrite(LINK_SYMBOL, sizeof(int8_t), rt_os_strlen(LINK_SYMBOL), fp);
            rt_fwrite(" ", sizeof(int8_t), 1, fp);
            rt_fwrite(value_array[i], sizeof(int8_t), rt_os_strlen(value_array[i]), fp);
            rt_fwrite("\n", sizeof(int8_t), 1, fp);
            rt_fwrite("\n", sizeof(int8_t), 1, fp);
        }
    }
    rt_fclose(fp);
    fp = NULL;
    return RT_SUCCESS;
}

/**
* ����ע�ͺͿհ�
* @params   fpp     �ļ�ָ���ָ��
* @return   �ɹ�����0�����򷵻�1
*/
static int32_t skip_annotation_and_spaces(rt_fshandle_t *fpp)
{
    int8_t temp[1] = {' '};
    int32_t char_size = sizeof(int8_t);
    int32_t annotation_head_len;
    int8_t *annotation_head = NULL;
    int32_t iResult;
    int32_t count = 0;

    annotation_head_len = rt_os_strlen(ANNOTATION_SYMBOL);
    annotation_head = rt_os_malloc(annotation_head_len + 1);
    if (NULL == annotation_head) {
        goto exit_entry;
    }

    while (1) {
        while (IS_SPACES(temp[0])) {
            iResult = rt_fread(temp, char_size, 1, *fpp);
            if (0 == iResult) {
                goto exit_entry;
            }
            count += 1;
        }
        rt_fseek(*fpp, 0 - char_size, RT_FS_SEEK_CUR);
        count -= 1;
        iResult = rt_fread(annotation_head, char_size, annotation_head_len, *fpp);
        if (annotation_head_len != iResult) {
            rt_fseek(*fpp, 0 - iResult*char_size, RT_FS_SEEK_CUR);
            goto exit_entry;
        }
        annotation_head[annotation_head_len] = '\0';

        if (0 != rt_os_strcmp(annotation_head, ANNOTATION_SYMBOL)){
            rt_fseek(*fpp, 0 - iResult*char_size, RT_FS_SEEK_CUR);
            goto exit_entry;
        }
        count += iResult;

        while (1) {
            iResult = rt_fread(temp, char_size, 1, *fpp);
            if (0 == iResult) {
                goto exit_entry;
            }
            count += 1;
            if ('\n' == temp[0]) {
                break;
            }
        }
        iResult = rt_fread(temp, char_size, 1, *fpp);
        if (0 == iResult){
            goto exit_entry;;
        }
        count += 1;
    }

exit_entry:

    if (annotation_head) {
        rt_os_free(annotation_head);
        annotation_head = NULL;
    }

    return count;
}


/**
* �������ļ��������ñ���Ϊ�ַ�����ʽ
* ��ȡ�����ȡʱ value ���ȳ����˶�Ӧ bufsize - 1 ������Զ��ض�
* ��������� value �ַ���
* ���Զ������հ׺�ע��
* @params   filePath        �����ļ�·��
* @params   key_array        ��ż������飬�� value_array һһ��Ӧ
* @params   value_array      ���ֵ�����飬�� key_array һһ��Ӧ
* @params   buf_size_array    ���ÿ�� value ��������С������
* @params   pair_num         key - value pair ������
* @return   �ɹ�����0�����򷵻�1
*/
static int32_t read_config_file(int8_t *file_path, const char **key_array, int8_t **value_array, int32_t buf_size_array, int32_t pair_num)
{
    rt_fshandle_t fp;    
    int32_t char_size = sizeof(int8_t);
    int32_t linkSymbolLen = rt_os_strlen(LINK_SYMBOL);
    int32_t i = 0;
    int32_t j = 0;
    int32_t is_end = 0;
    int32_t line_end = 0;
    int8_t temp[1];
    int32_t iResult = 0;
    int32_t tempLen;
    int8_t *temp_key_buf;
    int8_t *temp_link_buf;

    fp = rt_fopen(file_path, "rb");
    if (NULL == fp) {
        return RT_ERROR;
    }

    temp_key_buf = rt_os_malloc(buf_size_array + 1);
    temp_link_buf = rt_os_malloc(linkSymbolLen + 1);
    if (NULL == temp_key_buf || NULL == temp_link_buf) {
        rt_fclose(fp);
        rt_os_free(temp_key_buf);
        rt_os_free(temp_link_buf);
        return RT_ERROR;
    }

    // ÿһ��ѭ�����ö�����key��key���������е�ֵ���бȶԣ�ֱ�����������ļ�
    while (!is_end) {
        skip_annotation_and_spaces(&fp);
        for (i = 0; i < pair_num; i++) {
            if (key_array[i] == NULL) {
                break;
            }

            iResult = rt_fread(temp_key_buf, char_size, strlen(key_array[i]), fp);
            if (0 == iResult) {
                is_end = 1;
                break;
            }
            temp_key_buf[iResult] = '\0';
            if (rt_os_strlen((void *)key_array[i]) != iResult || 0 != rt_os_strcmp((void *)temp_key_buf, (void *)key_array[i])) {
                rt_fseek(fp, 0 - char_size*iResult, SEEK_CUR);
                continue;
            }

            // �����ȣ���Ҫ�ж������Ƿ�Ϊ���ӷ�����ֹ����ͬǰ׺�����
            skip_annotation_and_spaces(&fp);
            iResult += rt_fread(temp_link_buf, char_size, linkSymbolLen, fp);
            temp_link_buf[linkSymbolLen] = '\0';
            if (0 != rt_os_strcmp(temp_link_buf, LINK_SYMBOL)) {
                rt_fseek(fp, 0 - char_size*iResult, SEEK_CUR);
                continue;
            }
            skip_annotation_and_spaces(&fp);

            value_array[i][buf_size_array- 1] = '\0';
            for (j = 0; j < buf_size_array - 1; j++) {
                iResult = rt_fread(&value_array[i][j], char_size, 1, fp);
                if (0 == iResult) {
                        value_array[i][j] = '\0';
                        is_end = 1;
                        break;
                }

                // ��������˻��оͲ鿴ǰ���Ƿ��лس�������оͽ��س���Ϊ��β�����򽫻�����Ϊ��β
                if ('\n' == value_array[i][j]) {
                    line_end = 1;
                    if ('\r' == value_array[i][j - 1]) {
                        value_array[i][j - 1] = '\0';
                        break;
                    } else {
                        value_array[i][j] = '\0';
                        break;
                    }
                }
            }

            if (!line_end) {
                while (1) {
                    iResult = rt_fread(temp, char_size, 1, fp);
                    if (0 == iResult || '\n' == temp[0]) {
                        break;
                    }
                }
            } else {
                line_end = 0;
            }
            break;
        }

        while (1) {
            iResult = rt_fread(temp, char_size, 1, fp);
            if (0 == iResult || '\n' == temp[0]) {
                break;
            }
        }
    }

    rt_os_free(temp_key_buf);
    rt_os_free(temp_link_buf);
    rt_fclose(fp);
    return RT_SUCCESS;
}

/**
* ��ȡ�����ļ�
*/
static void parse_config_file(void)
{
    int8_t *value_p;

    read_config_file(CONFIG_FILE_PATH, keys, values, MAX_VALUE_SIZE, ARRAY_SIZE(keys));
    
    if (get_config_data(_DIS_CONNECT_WAIT_TIME, &value_p) == RT_SUCCESS)
        DIS_CONNECT_WAIT_TIME = msg_string_to_int(value_p);

    get_config_data(_OTI_ENVIRONMENT_ADDR, &OTI_ENVIRONMENT_ADDR);

    get_config_data(_EMQ_SERVER_ADDR, &EMQ_SERVER_ADDR);

    get_config_data(_PROXY_SERVER_ADDR, &PROXY_SERVER_ADDR);

    if (get_config_data(_LOG_FILE_SIZE, &value_p) == RT_SUCCESS) {
        LOG_FILE_SIZE = msg_string_to_int(value_p);
    }

    if (get_config_data(_MBN_CONFIGURATION, &value_p) == RT_SUCCESS) {
        MBN_CONFIGURATION = msg_string_to_int(value_p);
    }

    if (get_config_data(_INIT_PROFILE_TYPE, &value_p) == RT_SUCCESS) {
        INIT_PROFILE_TYPE = msg_string_to_int(value_p);
    }

    if (get_config_data(_RPLMN_ENABLE, &value_p) == RT_SUCCESS) {
        RPLMN_ENABLE = msg_string_to_int(value_p);
    }

    if (get_config_data(_UICC_MODE, &value_p) == RT_SUCCESS) {
        UICC_MODE = msg_string_to_int(value_p);
    }
}

/**
* ���������ļ�
*/
static void modify_config_file(void)
{
    int8_t buf[10];
    
    snprintf(buf, sizeof(buf), "%d", DIS_CONNECT_WAIT_TIME);
    set_config_data(_DIS_CONNECT_WAIT_TIME, buf);

    set_config_data(_OTI_ENVIRONMENT_ADDR, OTI_ENVIRONMENT_ADDR);
    set_config_data(_EMQ_SERVER_ADDR, EMQ_SERVER_ADDR);
    set_config_data(_PROXY_SERVER_ADDR, PROXY_SERVER_ADDR);

    snprintf(buf, sizeof(buf), "%d", MBN_CONFIGURATION);
    set_config_data(_MBN_CONFIGURATION, buf);

    snprintf(buf, sizeof(buf), "%d", LOG_FILE_SIZE);
    set_config_data(_LOG_FILE_SIZE, buf);

    snprintf(buf, sizeof(buf), "%d", INIT_PROFILE_TYPE);
    set_config_data(_INIT_PROFILE_TYPE, buf);

    snprintf(buf, sizeof(buf), "%d", RPLMN_ENABLE);
    set_config_data(_RPLMN_ENABLE, buf);

    snprintf(buf, sizeof(buf), "%d", UICC_MODE);
    set_config_data(_UICC_MODE, buf);

    write_config_file(CONFIG_FILE_PATH, keys, values, annotations, ARRAY_SIZE(keys));
    MSG_PRINTF(LOG_DBG, "Create default param for [%s] environment !!!\r\n", CFG_ENV_TYPE);
}

static void config_debug_cur_param(void)
{
    MSG_PRINTF(LOG_DBG, "RT_ENVIRONMENT_ADDR   : %s\n", OTI_ENVIRONMENT_ADDR);
    MSG_PRINTF(LOG_DBG, "EMQ_SERVER_ADDR       : %s\n", EMQ_SERVER_ADDR);
    MSG_PRINTF(LOG_DBG, "PROXY_SERVER_ADDR     : %s\n", PROXY_SERVER_ADDR);
    MSG_PRINTF(LOG_DBG, "DIS_CONNECT_WAIT_TIME : %d\n", DIS_CONNECT_WAIT_TIME);
    MSG_PRINTF(LOG_DBG, "LOG_FILE_SIZE         : %d\n", LOG_FILE_SIZE);
    MSG_PRINTF(LOG_DBG, "MBN_CONFIGURATION     : %d\n", MBN_CONFIGURATION);
    MSG_PRINTF(LOG_DBG, "INIT_PROFILE_TYPE     : %d\n", INIT_PROFILE_TYPE);
    MSG_PRINTF(LOG_DBG, "RPLMN_ENABLE          : %d\n", RPLMN_ENABLE);
    MSG_PRINTF(LOG_DBG, "UICC_MODE             : %s\n", (UICC_MODE == UICC_MODE_vUICC) ? "vUICC" : "eUICC");   
}

/**
* ��ȡ�������ֵ
* ���������ֵΪ���Σ���ôdata���ص����������ֵ��
* ����������ֵΪ�ַ��ͣ�data���ص���������ֵ�ĵ�ַ
* @params   config_type    ��������
* @params   data                ����ֵ
* @return   �ɹ����� RT_SUCCESS�����򷵻� RT_ERROR
*/
int32_t get_config_data(config_type_e config_type, int8_t **data)
{
    if (data == NULL) {
        return RT_ERROR;
    }

    if(values[config_type][0] != '\0') {
        *data = (values[config_type]);
        return RT_SUCCESS;
    }
    return RT_ERROR;
}

/**
* �޸��������ֵ
* @params   config_type    ��������
* @params   data                ����ֵ,Ϊstring����
* @return   �ɹ����� RT_SUCCESS�����򷵻� RT_ERROR
*/
int32_t set_config_data(config_type_e config_type, int8_t *data)
{
    if (NULL == data) {
        return RT_ERROR;
    }

    memcpy(values[config_type], data, rt_os_strlen(data));
    values[config_type][rt_os_strlen(data)] = '\0';
    return RT_SUCCESS;
}

int32_t init_config(void *arg)
{
    int32_t i;
    public_value_list_t *public_value_list = (public_value_list_t *)arg;

    for(i=0; i < ARRAY_SIZE(keys); i++) {
        values[i] = (int8_t *)rt_os_malloc( MAX_VALUE_SIZE * sizeof(int8_t));
        if (values[i] == NULL) {
            return RT_ERROR;
        }
        values[i][0] = '\0';
    }

    if (rt_os_access(CONFIG_FILE_PATH, F_OK) == RT_ERROR) {
        modify_config_file();
    } else {
        parse_config_file();
    }

    config_debug_cur_param();

    public_value_list->lpa_channel_type = (UICC_MODE == UICC_MODE_vUICC) ? LPA_CHANNEL_BY_IPC : LPA_CHANNEL_BY_QMI;
    MSG_PRINTF(LOG_DBG, "public_value_list->lpa_channel_type=%d\r\n", public_value_list->lpa_channel_type);

    return RT_SUCCESS;
}
