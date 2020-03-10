
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : vuicc_callback.c
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "vuicc_callback.h"

FILE_OPS_DEFINITION(linux)
OS_OPS_DEFINITION(linux)
MEM_OPS_DEFINITION(rt_os)

int init_callback_ops(void)
{
#if 0
    _file_ops_init(linux);
    _os_ops_init(linux);
    _mem_ops_init(rt_os);
#endif
    return RT_SUCCESS;
}
