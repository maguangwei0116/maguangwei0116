/*
 * upgrade.h
 *
 *  Created on: 2018å¹´11æœˆ26æ—¥
 *      Author: xiangyinglai
 */

#ifndef __INCLUDE_UPGRADE_H__
#define __INCLUDE_UPGRADE_H__

#include "stdint.h"
#include "rt_type.h"

#define MAX_DOWNLOAD_TIMEOUTS           120     // unit: second

typedef enum UPGRADE_MODE {
    UPGRADE_MODE_FORCED                 = 0,
    UPGRADE_MODE_CHK_FILE_NAME          = 1,
    UPGRADE_MODE_CHK_VERSION            = 2,
    UPGRADE_MODE_NO_FORCED              = 3,
} upgrade_mode_e;

typedef enum UPGRADE_PROFILE_TYPE {
    UPGRADE_PRO_TYPE_ANY                = 0,
    UPGRADE_PRO_TYPE_OPERATIONAL        = 1,
} upgrade_profile_type_e;

typedef enum TARGET_TYPE {
    TARGET_TYPE_AGENT                   = 0,
    TARGET_TYPE_SHARE_PROFILE           = 1,
    TARGET_TYPE_MONITOR                 = 2,
    TARGET_TYPE_COMM_SO                 = 3,
#ifdef CFG_STANDARD_MODULE
    TARGET_TYPE_OEMAPP                  = 4,    // oemapp.ubi
#endif
    TARGET_TYPE_MAX,
    TARGET_TYPE_DEF_SHARE_PROFILE       = 168,  // default share profile
} target_type_e;

typedef enum DEF_TARGET_TYPE {
    DEF_TARGET_TYPE_DEF_AGENT           = 0,
    DEF_TARGET_TYPE_DEF_SHARE_PROFILE   = 1,
} def_target_type_e;

/* OTA upgrade error code list */
typedef enum UPGRADE_RESULT {
    UPGRADE_NO_FAILURE                  = 0,
    UPGRADE_CHECK_VERSION_ERROR         = -2001,
    UPGRADE_DOWNLOAD_PACKET_ERROR       = -2002,
    UPGRADE_HASH_CHECK_ERROR            = -2003,
    UPGRADE_SIGN_CHECK_ERROR            = -2004,
    UPGRADE_INSTALL_APP_ERROR           = -2005,
    UPGRADE_FS_SPACE_NOT_ENOUGH_ERROR   = -2006,
    UPGRADE_DIR_PERMISSION_ERROR        = -2007,
    UPGRADE_START_UPGRADE_ERROR         = -2008,
    UPGRADE_FILE_NAME_ERROR             = -2009,
    UPGRADE_PROFILE_TYPE_ERROR          = -2010,
    UPGRADE_NULL_POINTER_ERROR          = -2011,    
    UPGRADE_EXECUTION_TYPE_ERROR        = -2012,
    UPGRADE_CHIP_MODEL_ERROR            = -2013,
    UPGRADE_TARGET_TYPE_ERROR           = -2014,
    UPGRADE_TARGET_TYPE_MATCH_ERROR     = -2015,
    UPGRADE_OTHER                       = -2099,
} upgrade_result_e;

typedef rt_bool (*file_check)(const void *arg);
typedef rt_bool (*file_install)(const void *arg);
typedef rt_bool (*file_cleanup)(const void *arg);
typedef rt_bool (*upload_on_event)(const void *arg);
typedef rt_bool (*file_activate)(const void *arg);

typedef struct UPGRADE_STRUCT {
#define MAX_TRANID_LEN                  128
#define MAX_CHIP_MODEL_LEN              32
#define MAX_VERSION_NAME_LEN            64
#define MAX_FILE_NAME_LEN               128
#define MAX_FILE_HASH_LEN               64
#define MAX_FILE_HASH_BYTE_LEN          32
#define MAX_TICKET_LEN                  32
#define MAX_UPLOAD_EVENT_LEN            32

/* ¸ÃºêÓÃÓÚÉèÖÃÔÚÏßÉý¼¶×´Ì¬±êÊ¾Îª */
#define SET_UPGRADE_FLAG(obj, update_mode, force_update) \
    (obj)->upgrade_flag = ((update_mode) | (force_update) << 3)

/* ¸ÃºêÓÃÓÚ»ñÈ¡ÔÚÏßÉý¼¶×´Ì¬±êÊ¾Îª */
#define GET_UPDATEMODE(obj)             (((obj)->upgrade_flag >> 0) & 0x03)
#define SET_UPDATEMODE(obj, data)       ((obj)->upgrade_flag |= (data))
#define GET_FORCEUPDATE(obj)            (((obj)->upgrade_flag >> 2) & 0x01)
#define SET_FORCEUPDATE(obj, data)      ((obj)->upgrade_flag |= (data) << 2)
#define GET_UPGRADE_STATUS(obj)         (((obj)->upgrade_flag >> 3) & 0x01)
#define SET_UPGRADE_STATUS(obj, data)   ((obj)->upgrade_flag |= (data) << 3)

/* download result code */
#define SET_DOWNLOAD_RET(upgrade, ret)  ((upgrade_struct_t *)(upgrade))->downloadResult = ret
#define GET_DOWNLOAD_RET(upgrade, ret)  ret = ((upgrade_struct_t *)(upgrade))->downloadResult

    uint8_t         upgrade_flag;
    /* ÔÚÏßÉý¼¶Ïà¹Ø²ÎÊý±êÖ¾Î»
     * -bit0-1--updateMode£¬1ÎªÈ«Á¿¸üÐÂ£¬2ÎªFOTA¸üÐÂ
     * -bit2--ÊÇ·ñÖ§³Ö½µ¼¶²Ù×÷
     * -bit3--Éý¼¶ÊÇ·ñ³É¹¦£¬1³É¹¦£¬0Ê§°Ü
     */

    char            tranId[MAX_TRANID_LEN + 1];
    char            targetName[MAX_FILE_NAME_LEN + 1];
    char            targetVersion[MAX_VERSION_NAME_LEN + 1];
    char            targetChipModel[MAX_CHIP_MODEL_LEN + 1];
    char            targetFileName[MAX_FILE_NAME_LEN + 1];  // the full path in local file system
    char            tmpFileName[MAX_FILE_NAME_LEN + 1];     // the full path in local file system  
    char            fileHash[MAX_FILE_HASH_LEN + 1];        // hash code of the upgrade file
    char            ticket[MAX_TICKET_LEN + 1];
    uint8_t         type;
    uint32_t        size;
    uint16_t        retryAttempts;
    uint16_t        retryInterval;
    int32_t         downloadResult;                         // the result of download process
    rt_bool         execute_app_now;                        // execute app right now after install app
    char            event[MAX_UPLOAD_EVENT_LEN + 1];

    /* callback functions */
    file_check      check;
    file_install    install;
    file_cleanup    cleanup;
    upload_on_event on_event;
    file_activate   activate;

#ifdef CFG_STANDARD_MODULE
    /* ubi update */
    rt_bool         ubi;
#endif
} upgrade_struct_t;

int32_t upgrade_process_create(upgrade_struct_t **d_info);
int32_t upgrade_process_start(upgrade_struct_t *d_info);
int32_t init_upgrade(void * arg);

#endif /* __INCLUDE_UPGRADE_H__ */
