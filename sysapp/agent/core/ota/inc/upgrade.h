/*
 * upgrade.h
 *
 *  Created on: 2018年11月26日
 *      Author: xiangyinglai
 */

#ifndef __INCLUDE_UPGRADE_H__
#define __INCLUDE_UPGRADE_H__

#include "stdint.h"
#include "rt_type.h"

#define MAX_DOWNLOAD_TIMEOUTS           120     // unit: second
#define MAX_FILE_PATH_LEN               100
#define TMP_DOWNLOAD_PATH               "/data/"
#define BACKUP_INFO_FILE                "/data/rt_upgrade_info"

typedef enum {
    UPGRADE_NO_FAILURE = 0,
    UPGRADE_CHECK_VERSION_ERROR         = -2001,
    UPGRADE_DOWNLOAD_PACKET_ERROR       = -2002,
    UPGRADE_CHECK_PACKET_ERROR          = -2003,
    UPGRADE_INSTALL_APP_ERROR           = -2004,
    UPGRADE_SAVE_INFO_ERROR             = -2005,
    UPGRADE_FS_SPACE_NOT_ENOUGH_ERROR   = -2006,
    UPGRADE_DIR_PERMISSION_ERROR        = -2007,
    UPGRADE_OTHER                       = -2010,
} rt_upgrade_result_e;

typedef struct upgrade_struct {
#define MAX_TRANID_LEN                  128
#define MAX_MAKE_LEN                    32
#define MAX_CHIP_MODEL_LEN              32
#define MAX_VERSION_NAME_LEN            128
#define MAX_FILE_NAME_LEN               128
#define MAX_FILE_HASH_LEN               64
#define MAX_TICKET_LEN                  32
#define HASH_CHECK_BLOCK                1024  // ��ϣУ��ÿ��Ĵ�С

/* �ú�����������������״̬��ʾΪ */
#define SET_UPGRADE_FLAG(obj, update_mode, force_update) \
    (obj)->upgrade_flag = ((update_mode) | (force_update) << 3)

/* �ú����ڻ�ȡ��������״̬��ʾΪ */
#define GET_UPDATEMODE(obj)             (((obj)->upgrade_flag >> 0) & 0x03)
#define SET_UPDATEMODE(obj, data)       ((obj)->upgrade_flag |= (data))
#define GET_FORCEUPDATE(obj)            (((obj)->upgrade_flag >> 2) & 0x01)
#define SET_FORCEUPDATE(obj, data)      ((obj)->upgrade_flag |= (data) << 2)
#define GET_UPGRADE_STATUS(obj)         (((obj)->upgrade_flag >> 3) & 0x01)
#define SET_UPGRADE_STATUS(obj, data)   ((obj)->upgrade_flag |= (data) << 3)

/* lock for download process */
#define DOWNLOAD_LOCKED                 1
#define DOWNLOAD_UNLOCKED               0
#define DOWNLOAD_LOCK(upgrade)          ((upgrade_struct_t *)(upgrade))->downloadLock = DOWNLOAD_LOCKED
#define DOWNLOAD_UNLOCK(upgrade)        ((upgrade_struct_t *)(upgrade))->downloadLock = DOWNLOAD_UNLOCKED
#define DOWNLOAD_LOCK_CHECK(upgrade)    (((upgrade_struct_t *)(upgrade))->downloadLock == DOWNLOAD_LOCKED)

/* download result code */
#define SET_DOWNLOAD_RET(upgrade, ret)  ((upgrade_struct_t *)(upgrade))->downloadResult = ret
#define GET_DOWNLOAD_RET(upgrade, ret)  ret = ((upgrade_struct_t *)(upgrade))->downloadResult

    int8_t      upgrade_flag;
    /* ����������ز�����־λ
     * -bit0-1--updateMode��1Ϊȫ�����£�2ΪFOTA����
     * -bit2--�Ƿ�֧�ֽ�������
     * -bit3--�����Ƿ�ɹ���1�ɹ���0ʧ��
     */

    int8_t      tranId[MAX_TRANID_LEN + 1];
    int8_t      make[MAX_MAKE_LEN + 1];
    int8_t      versioncode;
    int8_t      chipModel[MAX_CHIP_MODEL_LEN + 1];
    int8_t      versionName[MAX_VERSION_NAME_LEN + 1];
    int8_t      targetFileName[MAX_FILE_NAME_LEN + 1];  // the full path in local file system
    int8_t      fileName[MAX_FILE_NAME_LEN + 1];        // file name of push file name, such as "linux-euicc-agent-general"
    int8_t      fileHash[MAX_FILE_HASH_LEN + 1];        // hash code of the upgrade file
    int8_t      ticket[MAX_TICKET_LEN + 1];
    int8_t      buffer[HASH_CHECK_BLOCK];
    uint16_t    retryAttempts;
    uint16_t    retryInterval;
    uint8_t     downloadLock;                           // lock for download process
    int32_t     downloadResult;                         // the result of download process
    rt_bool     excute_app_now;                         // excute app right now after install app
} upgrade_struct_t;

int32_t upgrade_process_create(upgrade_struct_t **d_info);
int32_t upgrade_process_start(upgrade_struct_t *d_info);
int32_t upgrade_process_wating(upgrade_struct_t *d_info, int32_t timeous);

#endif /* __INCLUDE_UPGRADE_H__ */
