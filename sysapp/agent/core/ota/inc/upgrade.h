/*
 * upgrade.h
 *
 *  Created on: 2018年11月26日
 *      Author: xiangyinglai
 */

#ifndef __INCLUDE_UPGRADE_H__
#define __INCLUDE_UPGRADE_H__

#include "stdint.h"

#define MAX_FILE_PATH_LEN                  100
#define BACKUP_PATH                        "/data/"
#define AGENT_PATH                         "/usr/bin/agent"
#define BACKUP_INFO_FILE                   "/data/rt_upgrade_info"


typedef enum {
    UPGRADE_NO_FAILURE = 0,
    UPGRADE_VERSION_NUM_ERROR = 2001,
    UPGRADE_DOWNLOAD_PACKET_ERROR = 2002,
    UPGRADE_CHECK_PACKET_ERROR = 2003,
    UPGRADE_REPLACE_APP_ERROR = 2004,
    UPGRADE_SAVE_INFO_ERROR = 2005,
    UPGRADE_OTHER = 2010
} rt_upgrade_result_e;

typedef struct upgrade_struct {
#define MAX_TRANID_LEN              128
#define MAX_MAKE_LEN                32
#define MAX_VERSION_NAME_LEN        128
#define MAX_FILE_NAME_LEN           128
#define MAX_FILE_HASH_LEN           64
#define MAX_TICKET_LEN              32
#define HASH_CHECK_BLOCK            1024  // ��ϣУ��ÿ��Ĵ�С

/* �ú�����������������״̬��ʾΪ */
#define SET_UPGRADE_FLAG(obj, update_mode, force_update) \
    (obj)->upgrade_flag = ((update_mode) | (force_update) << 3)

/* �ú����ڻ�ȡ��������״̬��ʾΪ */
#define GET_UPDATEMODE(obj)                      (((obj)->upgrade_flag >> 0) & 0x03)
#define SET_UPDATEMODE(obj, data)                ((obj)->upgrade_flag |= (data))
#define GET_FORCEUPDATE(obj)                     (((obj)->upgrade_flag >> 2) & 0x01)
#define SET_FORCEUPDATE(obj, data)               ((obj)->upgrade_flag |= (data) << 2)
#define GET_UPGRADE_STATUS(obj)                  (((obj)->upgrade_flag >> 3) & 0x01)
#define SET_UPGRADE_STATUS(obj, data)            ((obj)->upgrade_flag |= (data) << 3)

    int8_t upgrade_flag;
    /* ����������ز�����־λ     
     * -bit0-1--updateMode��1Ϊȫ�����£�2ΪFOTA����     
     * -bit2--�Ƿ�֧�ֽ�������     
     * -bit3--�����Ƿ�ɹ���1�ɹ���0ʧ��     
     */

    int8_t tranid[MAX_TRANID_LEN + 1];
    int8_t make[MAX_MAKE_LEN + 1];
    int8_t versioncode;  // �汾��ʶ
    int8_t versionName[MAX_VERSION_NAME_LEN + 1];
    int8_t fileName[MAX_FILE_NAME_LEN + 1];
    int8_t fileHash[MAX_FILE_HASH_LEN + 1];  // ƽ̨�����ļ���hash��
    int8_t ticket[MAX_TICKET_LEN + 1];
    int8_t buffer[HASH_CHECK_BLOCK];
} upgrade_struct_t;

extern void * check_upgrade_process(void *args);
extern void upgrade_check_info(void);

#endif /* __INCLUDE_UPGRADE_H__ */
