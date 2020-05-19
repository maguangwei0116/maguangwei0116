/*****************************************************************************
Filename: callback.h
Author  :
Date    : 2019-10-16 9:17:22
Description:
*****************************************************************************/
#ifndef __CB_IO_H__
#define __CB_IO_H__

#include <stdint.h>
#include "cos_api.h"
#include "rt_type.h"

int32_t linux_io_server_init(void);
int32_t linux_io_server_wait(uint32_t timeout, io_id_t *io_id);
int32_t linux_io_server_send(io_id_t io_id, uint8_t *src, uint16_t length);
int32_t linux_io_server_recv(io_id_t io_id, uint8_t *dest, uint16_t length);

int32_t linux_io_client_connect(void);
int32_t linux_io_client_close(void);
int32_t linux_io_client_send(uint8_t *src, uint16_t length);
int32_t linux_io_client_recv(uint8_t *dest, uint16_t length);

#endif /* __RT_CALLBACK__ */
