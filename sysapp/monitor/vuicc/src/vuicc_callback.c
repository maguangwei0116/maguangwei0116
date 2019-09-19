
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

int init_file_ops(void)
{
    return _file_ops_init(linux);
}
