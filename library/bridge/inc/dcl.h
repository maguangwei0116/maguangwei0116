/* Copyright Statement:
*
* (C) 2005-2017  MediaTek Inc. All rights reserved.
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
* Without the prior written permission of MediaTek and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
* You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
* if you have agreed to and been bound by the applicable license agreement with
* MediaTek ("License Agreement") and been granted explicit permission to do so within
* the License Agreement ("Permitted User").  If you are not a Permitted User,
* please cease any access or use of MediaTek Software immediately.
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
* ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#ifndef __DCL_H__
#define __DCL_H__


typedef char              DCL_CHAR;
typedef signed char       DCL_INT8;
typedef signed short      DCL_INT16;
typedef signed long       DCL_INT32;

typedef unsigned char      DCL_UCHAR;
typedef unsigned char      DCL_UINT8;
typedef unsigned short     DCL_UINT16;
typedef unsigned long      DCL_UINT32;

#if !defined(GEN_FOR_PC) && defined(_MSC_VER)
   typedef signed __int64    DCL_INT64;
typedef unsigned __int64   DCL_UINT64;
#else
   typedef unsigned long long   DCL_UINT64;
   typedef signed long long     DCL_INT64;
#endif

typedef double             DCL_DOUBLE;
typedef int   DCL_INT;

//typedef int DCL_BOOL;
typedef enum
{
   DCL_FALSE = 0,
   DCL_TRUE
}DCL_BOOLEAN;

#define DCL_BOOL  DCL_BOOLEAN


/*******************************************************************************
 * DCL_STATUS
 *******************************************************************************/
#if defined(STATUS_OK)
#undef STATUS_OK
#endif

#if defined(STATUS_FAIL)
#undef STATUS_FAIL
#endif

typedef DCL_INT32 DCL_STATUS;
typedef enum
{
   STATUS_OK = 0,
   STATUS_FAIL = -1,
   STATUS_INVALID_CMD = -2,
   STATUS_UNSUPPORTED = -3,
   STATUS_NOT_OPENED = -4,
   STATUS_INVALID_EVENT = -5,
   STATUS_INVALID_DCL_HANDLE = -6,
   STATUS_INVALID_CTRL_DATA = -7,
   STATUS_INVALID_CONFIGURATION = -8,
   STATUS_INVALID_ARGUMENT = -9,
   STATUS_ERROR_TIMEOUT = -10,
   STATUS_ERROR_CRCERROR = -11,
   STATUS_ERROR_READONLY = -12,
   STATUS_ERROR_WRONG_STATE = -13,
   STATUS_INVALID_DEVICE = -14,
   STATUS_ALREADY_OPENED = -15,
   STATUS_SET_VFIFO_FAIL = -16,
   STATUS_INVALID_OPERATION = -17,
   STATUS_DEVICE_NOT_EXIST = -18,

   //*/ I2C DCL added /*//
   STATUS_DEVICE_NOT_SUPPORT_DMA = -19,
   STATUS_DEVICE_IS_BUSY = -20,
   STATUS_ACKERR = -21,
   STATUS_HS_NACKERR = -22,

   STATUS_BUFFER_EMPTY = 1
} DCL_STATUS_T;

#endif  /* __DCL_H__ */
