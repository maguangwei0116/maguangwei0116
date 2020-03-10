/*****************************************************************************
Filename: cos_apilh
Author  :
Date    : 2020-02-17 21:52:22
Description: Provide some interfaces for the upper application calls
*****************************************************************************/
#ifndef __COS_API_H__
#define __COS_API_H__

#include <stdio.h>
#include <stdint.h>

/* 以下为IO_ID的定义 */
#ifndef IO_INTERFACE_INVALID
#define IO_INTERFACE_INVALID    0xFF
#endif

#ifndef IO_MAX_INTERFACES
#define IO_MAX_INTERFACES       2
#endif

#ifndef NVM_INIT_VALUE
#define NVM_INIT_VALUE              0xFF  // NVM默认填充值
#endif

#ifndef NVM_SIZE
#define NVM_SIZE                    0x5000UL  //NVM块总大小
#endif

/* 以下为io_type的定义*/
#define IO_PACKET_TYPE_DATA     0x7A
#define IO_PACKET_TYPE_CTRL     0x7B

#ifndef io_id_t
#define io_id_t int
#endif

#ifndef RESULE_T
#define RESULE_T
typedef int result_t;
#endif


typedef void (_cos_log_print)(const uint8_t *buf, uint32_t length);
typedef void *(_cos_memset)(void*, int, long int);
typedef void *(_cos_memcpy)(void*, const void*, long int);
typedef int32_t (_cos_memcmp)(void *, void *, uint32_t);
typedef void *(_cos_malloc)(uint32_t);
typedef void *(_cos_calloc)(uint32_t, uint32_t);
typedef void *(_cos_realloc)(void *, uint32_t);
typedef void (_cos_free)(void *);
typedef int32_t (_cos_strlen)(void *);
typedef uint32_t (_random)(uint32_t);

/* NVM需要实现内存块的读写能力，内存块大小由NVM_SIZE定义，内存默认填充值为NVM_INIT_VALUE */
typedef result_t (_nvm_init)(void);  // nvm_table_init在何处调用。
typedef result_t (_nvm_write)(const uint8_t *src, uint32_t offset, uint32_t length);
typedef result_t (_nvm_read)(const uint8_t *dest, uint32_t offset, uint32_t length);

/* IO需要实现客户端通服务器进行通信的功能更，且具备权限校验功能。*/
typedef result_t (_io_init)(void);
typedef result_t (_io_wait)(uint32_t timeout, io_id_t *io_id);
typedef io_id_t (_io_connct)(void);
typedef int32_t (_io_send)(io_id_t io_id, uint8_t *dest, uint16_t length);
typedef int32_t (_io_recv)(io_id_t io_id, uint8_t *dest, uint16_t length);
typedef void (_io_close)(io_id_t io_id);


typedef struct cos_type_operation {
    _cos_log_print *print;
    _cos_memset *memset;
    _cos_memcpy *memcpy;
    _cos_memcmp *memcmp;
    _cos_malloc *malloc;
    _cos_calloc *calloc;
    _cos_realloc *realloc;
    _cos_free *free;
    _cos_strlen *strlen;
    _random *random;
    _nvm_init *nvm_init;
    _nvm_write *nvm_write;
    _nvm_read *nvm_read;
    _io_init *io_init;
    _io_wait *io_wait;
    _io_connct *io_connct;
    _io_send *io_send;
    _io_recv *io_recv;
    _io_close *io_close;
} cos_type_operation_t;


/*
 * This function is used to start cos task and initialize the cos operation function
 * PARAMETER：
 *   op：The operating function of used to register if is NULL, cos use the default implementation
 * RETURN VALUES:
 *   0: The structure is printed.
 *  -1: Problem dumping the structure.
 */
extern result_t cos_init(cos_type_operation_t *op);
extern result_t cos_get_ver(io_id_t io_id, uint8_t *ver, uint16_t *ver_len);
extern result_t cos_power_off(io_id_t io_id);
extern result_t cos_power_on(io_id_t io_id, uint8_t *atr, uint16_t *atr_len);

/*
 * This function is used to connect cos task
 * PARAMETER：
 *   arg: information parameter
 * RETURN VALUES:
 *   >=0: The io id which used to communicate with cos
 *  <0: false
 */
extern io_id_t cos_client_connect(void *arg);

/*
 * This function is used to reset cos task
 * PARAMETER：
 *   atr: return atr of cos
 *   atr_len: atr atr length
 * RETURN VALUES:
 *   0: The structure is printed.
 *  -1: Problem dumping the structure.
 */
extern result_t cos_client_reset(io_id_t io_id, uint8_t *atr, uint16_t *atr_len);

/*
 * This function is used to send command to cos task
 * PARAMETER：
 *   io_id: the io client id 0 is the modem and 1 is the lpa
 *   request: request data struct
 *   responds: responds data struct
 * RETURN VALUES:
 *   0: The structure is printed.
 *  -1: Problem dumping the structure.
 */
extern result_t cos_client_transport(io_id_t io_id, uint8_t io_type, uint8_t *req, uint16_t req_len, uint8_t *resp, uint16_t *resp_len);

/*
 * This function is used to close communicate with the cos
 * PARAMETER：
 *   io_id：the io_id whihc need to close
 */
extern void cos_client_close(io_id_t io_id);
#endif /* __COS_API_H__ */
