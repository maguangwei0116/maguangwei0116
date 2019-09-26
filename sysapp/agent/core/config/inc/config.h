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

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
#endif

typedef enum CONFIG_TYPE {
    _OTI_ENVIRONMENT_ADDR,
    _EMQ_SERVER_ADDR,
    _PROXY_SERVER_ADDR,
    _MBN_CONFIGURATION,
    _LOG_FILE_SIZE,
    _INIT_PROFILE_TYPE,
    _RPLMN_ENABLE,
    _UICC_MODE
} config_type_e;

typedef enum UICC_MODE {
    UICC_MODE_vUICC         = 0,
    UICC_MODE_eUICC         = 1,
} uicc_mode_e;

extern int8_t *OTI_ENVIRONMENT_ADDR;
extern int8_t *EMQ_SERVER_ADDR;
extern int8_t *PROXY_SERVER_ADDR;
extern int32_t MBN_CONFIGURATION;
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
