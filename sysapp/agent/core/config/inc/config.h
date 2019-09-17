/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_config.h
 * Date        : 2017.09.01
 * Note        :
 * Description : The configuration file is used to configure file function

 *******************************************************************************/
#ifndef __RT_CONFIG_H__
#define __RT_CONFIG_H__

#include "stdint.h"


/************************************debug***********************************/

/************************************fallback***********************************/
#define DEFAULT_DIS_CONNECT_WAIT_TIME           100  // Ĭ��fallbackΪ5��
// #define DEFAULT_SEED_CARD_FIRST                 0  // Ĭ�ϲ������ӿ�����

/************************************general***********************************/
#define DEFAULT_OTI_ENVIRONMENT_ADDR            "52.220.34.227"  // Ĭ����������
#define DEFAULT_OTI_ENVIRONMENT_PORT            7082
#define DEFAULT_EMQ_SERVER_ADDR                 "18.136.190.97"  // Ĭ����������EMQ��ַ
#define DEFAULT_PROXY_SERVER_ADDR               "smdp.redtea.io"  //Ĭ����������smdp��ַ
#define DEFAULT_CARD_TYPE_FLAG                  1  // �Ƿ�����/data/card_type�ļ�
#define DEFAULT_MBN_CONFIGURATION               1  // Ĭ�Ͽ���MBN���ù���
#define DEFAULT_LOG_FILE_SIZE                   1  // Ĭ��log��СΪ1M
#define DEFAULT_MBN_CONFIGURATION               1  // Ĭ�Ͽ���MBN���ù���
#define DEFAULT_INIT_PROFILE_TYPE               2  // Ĭ��������һ�ſ�
#define DEFAULT_RPLMN_ENABLE                    1  //Ĭ�Ͽ���rplmn���ù���
#define DEFAULT_UICC_MODE                       UICC_MODE_eUICC  //Ĭ��ʹ��QMIͨ������ʵ�忨ģʽ(eUICC)

/********************************platform**************************************/

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
#endif

typedef enum config_type {
    _DIS_CONNECT_WAIT_TIME  = 0,
    _OTI_ENVIRONMENT_ADDR,
    _EMQ_SERVER_ADDR,
    _PROXY_SERVER_ADDR,
    _MBN_CONFIGURATION,
    _LOG_FILE_SIZE,
    _INIT_PROFILE_TYPE,
    _RPLMN_ENABLE,
    _UICC_MODE
} config_type_e;

typedef enum _uicc_mode_e {
    UICC_MODE_vUICC         = 0,
    UICC_MODE_eUICC         = 1,
} uicc_mode_e;

extern int32_t DIS_CONNECT_WAIT_TIME;
extern int8_t *OTI_ENVIRONMENT_ADDR;
extern int8_t *EMQ_SERVER_ADDR;
extern int8_t *PROXY_SERVER_ADDR;
extern int32_t MBN_CONFIGURATION;
extern int32_t LOG_FILE_SIZE;
extern int32_t INIT_PROFILE_TYPE;
extern int32_t RPLMN_ENABLE;
extern int32_t UICC_MODE;

int32_t init_config(void *arg);

/**
* ��ȡ�������ֵ
* ���������ֵΪ���Σ���ôdata���ص����������ֵ��
* ����������ֵΪ�ַ��ͣ�data���ص���������ֵ�ĵ�ַ
* @params   config_type    ��������
* @params   data                ����ֵ
* @params   data_type       0��ʾ���Σ�1��ʾ�ַ���
* @return   �ɹ����� RT_SUCCESS�����򷵻� RT_ERROR
*/
int32_t get_config_data(config_type_e config_type, int8_t **data);
int32_t set_config_data(config_type_e config_type, int8_t *data);

#endif  // __RT_CONFIG_H__
