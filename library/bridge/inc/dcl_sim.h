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

#ifndef __DCL_SIM_H_STRUCT__
#define __DCL_SIM_H_STRUCT__

#include "dcl.h"

#define DCL_SIM_MAX_INTERFACE 5

/*********************************************************************************************************
*followings we defines the structure used by DCL user, this is prevent DCL user from include invividual moudles' header file.
**********************************************************************************************************/
typedef DCL_UINT16 DCL_SIM_STATUS_WORD;

typedef enum{
  DCL_USIM_NO_ERROR = 0,

  /*expected status*/
  DCL_USIM_WAITING_EVENT = 1, /* initial wait event status */
  DCL_USIM_BLOCK_REC = 2,   /* successfully received a complete block */
  DCL_USIM_POWER_OFF = 3,   /* successfully powered off */
  DCL_USIM_ATR_REC = 4,     /* successfully reveived all ATR */
  DCL_USIM_S_BLOCK_REC = 5,   /* successfully reveived S RESP */

  /* error status */
  DCL_USIM_NO_INSERT = -1,
  DCL_USIM_VOLT_NOT_SUPPORT = -2,
  DCL_USIM_NO_ATR = -3,
  DCL_USIM_TS_INVALID = -4,
  DCL_USIM_ATR_ERR = -5,
  DCL_USIM_INVALID_ATR = -6,
  DCL_USIM_PTS_FAIL = -7,
  DCL_USIM_RX_INVALID = -8, /* EDC error or parity error */
  DCL_USIM_BWT_TIMEOUT = -9,
  DCL_USIM_DATA_ABORT = -10,
  DCL_USIM_DEACTIVATED = -11,
  DCL_USIM_S_BLOCK_FAIL = -12,
  DCL_USIM_INVALID_WRST = -13,
  DCL_USIM_GPT_TIMEOUT = -14
}DCL_SIM_STATUS;

/*All possible volatege support values. This enum to duplicate the same definition as internal SIM driver definition, so that protocol does not need to include driver internal header files*/
typedef enum{
  DCL_UNKNOWN_POWER_CLASS = 0,
  DCL_CLASS_A_50V = 1,              /*this card support 5V only, from ATR*/
  DCL_CLASS_B_30V = 2,              /*this card support 3V only, from ATR*/
  DCL_CLASS_AB  = 3,                /*this card support 5V and 3V, from ATR*/
  DCL_CLASS_C_18V = 4,              /*this card support 1.8V only, from ATR*/
  DCL_ClASS_BC    = 6,              /*this card support 3V and 1.8V, from ATR*/
  DCL_CLASS_ABC = 7                 /*this card support 5V, 3V, and 1.8V, from ATR*/
}DCL_SIM_POWER;

typedef DCL_UINT32 DCL_SIM_HW_CB;

/*The application protocol support information. This enum to duplicate the same definition as internal SIM driver definition, so that protocol does not need to include driver internal header files*/
typedef enum
{
  DCL_SIM_PROTOCOL,     /*support SIM protocol, or to be more precisely, this should be a ICC card*/
  DCL_USIM_PROTOCOL     /*support USIM protocol, or to be more precisely, this should be a UICC card*/
}DCL_SIM_APP_PROTOCOL;

/*defines the possible choice of physical protocol. This enum to duplicate the same definition as internal SIM driver definition, so that protocol does not need to include driver internal header files*/
typedef enum
{
  DCL_T0_PROTOCOL,      /*prefer to use the T0 protocol*/
  DCL_T1_PROTOCOL,      /*prefer to use the T1 protocol*/
  DCL_UNKNOWN_PROTOCOL  /*no preferred physical protocol*/
}DCL_SIM_PHY_PROTOCOL;

/*defines the all of the clock stop related information. This enum to duplicate the same definition as internal SIM driver definition, so that protocol does not need to include driver internal header files*/
typedef enum{
  DCL_CLOCK_STOP_NOT_SUPPORT  = 0x0,    /*not support clock stop*/
  DCL_CLOCK_STOP_LOW      = 0x40,       /*support clock stop in low level*/
  DCL_CLOCK_STOP_HIGH       = 0x80,     /*support clock stop in high level*/
  DCL_CLOCK_STOP_ANY        = 0xc0,     /*support clock stop in any voltage level*/
  DCL_CLOCK_STOP_MSK        = 0xc0,     /*define the clock stop mask, used to parse the information received from cards*/
  DCL_CLOCK_STOP_UNKONW     = 0x0f      /*the default value*/
}DCL_SIM_CLK_STOP;

/*defines the possible card speed values. This enum is to duplicate the same definition as internal SIM driver definition, so that protocol does not need to include driver internal header files*/
typedef enum{
  DCL_SPEED_372,    /*for speed372*/
  DCL_SPEED_64,     /*for speed64*/
  DCL_SPEED_32,     /*for speed32*/
  DCL_SPEED_16      /*for speed16*/
}DCL_SIM_CARD_SPEED;

/*the structure to define the values exchanged when doing get card information*/
typedef struct{
  DCL_SIM_POWER power; /*the voltage we choose*/
  DCL_SIM_CARD_SPEED speed; /*the speed we choose*/
  DCL_SIM_CLK_STOP clock_stop; /*clock stop information in ATR*/
  DCL_SIM_APP_PROTOCOL app_proto; /*whether it is a ICC card or UICC card*/
  DCL_SIM_PHY_PROTOCOL phy_proto; /*the physical protocol we used*/
  DCL_BOOL T0_support;  /* if T0 is supported*/
  DCL_BOOL T1_support;  /* if T1 is supported*/
  DCL_UINT8 hist_index; /* index to the historical char of ATR*/
  DCL_UINT8 *ATR; /*the pointer to store the ATR received*/
  DCL_BOOL TAiExist; /*if the first TA for T=15 is existed*/
  DCL_UINT8 ATR_length;     /* length to the ATR_data*/
  DCL_BOOL isSW6263; /* query if status word 0x62xx 0x63xx happen*/
  DCL_UINT8 TB15; // query if support ic usb
  DCL_BOOL hasPowerClass;
  DCL_UINT8 PowerClass;
}DCL_SIM_INFO;

/*******************************************************************************
 * driver function tables exposed to DCL
 *******************************************************************************/
/*here are type definition for functions*/
/*dcl add new api : 2. define function type*/

/*the prototype definition for RST function pointer*/
typedef DCL_SIM_STATUS (* DCL_SIM_RST)(DCL_SIM_POWER ExpectVolt, DCL_SIM_POWER *ResultVolt, DCL_BOOL warm, DCL_SIM_HW_CB handle);
/*the prototype definition for CMD function pointer*/
typedef DCL_SIM_STATUS_WORD (* DCL_SIM_CMD)(DCL_UINT8 *txData, DCL_UINT32 *txSize, DCL_UINT8 *rxData, DCL_UINT32 *rxSize, DCL_SIM_HW_CB handle);
/*the prototype definition for power off function pointer*/
typedef void (* DCL_SIM_PWOFF)(DCL_SIM_HW_CB handle);
/*the prototype definition for get card info function pointer*/
typedef void (* DCL_SIM_GET_CARD_INFO)(DCL_SIM_INFO *info, DCL_SIM_HW_CB handle);
/*the prototype definition for set maximum speed function pointer*/
typedef void (* DCL_SIM_SET_MAX_SPEED)(DCL_SIM_CARD_SPEED speed, DCL_SIM_HW_CB handle);
/*the prototype definition for set prefer physical protocol function pointer*/
typedef void (* DCL_SIM_SET_PREFER_PROTOCOL)(DCL_SIM_PHY_PROTOCOL T, DCL_SIM_HW_CB handle);
/*the prototype definition for set clock stop mode function pointer*/
typedef DCL_BOOL (* DCL_SIM_SET_CLK_STOP_MODE)(DCL_SIM_CLK_STOP mode, DCL_SIM_HW_CB handle);
/*the prototype definition for TOUT test function pointer*/
typedef void (* DCL_SIM_TOUT_TEST)(DCL_UINT32 toutValue, DCL_SIM_HW_CB handle);
/*the prototype definition for assign logical number function pointer*/
typedef DCL_UINT32 (* DCL_SIM_ASSIGN_LOGICAL)(DCL_UINT32 logicalNumber);
/*the prototype definition for get card speed function pointer*/
typedef DCL_SIM_CARD_SPEED (*DCL_SIM_GAT_CARD_SPEED)(DCL_SIM_HW_CB handle);
/*the prototype definition for query 9000 function pointer*/
typedef DCL_BOOL (*DCL_SIM_QUERY_GET_9000_WHEN_SELECT)(DCL_SIM_HW_CB handle);
/* For sim hot plug callback function */
/*the prototype definition for SIM hot plug out call back function*/
typedef void (*DCL_SIM_PLUG_OUT_CALLBACK)(DCL_UINT32 simIf);
/*the prototype definition for SIM hot plug in call back function*/
typedef void (*DCL_SIM_PLUG_IN_CALLBACK)(DCL_UINT32 simIf);

/*dcl add new api : 3. add to funtion table type*/

/*the declaration of SIM driver table, expeorted from SIM driver to DCL SIM interface. This table implies we accept any SIM driver function tables with the same format.*/
typedef struct
{
  DCL_SIM_RST rst; /*The RST function to detect card's existence*/
  DCL_SIM_CMD cmd; /*The CMD function to send the APDU command to the card*/
  DCL_SIM_PWOFF pwOff; /*The power off function to deactivate the card*/
  DCL_SIM_GET_CARD_INFO getCardInfo; /*To report the card information, known by SIM driver, to the protocol layer*/
  DCL_SIM_SET_MAX_SPEED setSpeed; /*To set the maximum allowed speed to driver*/
  DCL_SIM_SET_PREFER_PROTOCOL setPreferT; /*To set the preferred physical layer protocol to driver*/
  DCL_SIM_SET_CLK_STOP_MODE setClockStopMode; /*To set the CLK stop information, known by protocol, to driver*/
  DCL_SIM_TOUT_TEST toutTest; /*To ask the driver to start TOUT test*/
  DCL_SIM_ASSIGN_LOGICAL assignLogical; /*To Assign the logical number to specific card*/
  DCL_SIM_GAT_CARD_SPEED getCardSpeed; /*to get the card speed information*/
  DCL_SIM_QUERY_GET_9000_WHEN_SELECT query9000WhenSelect; /*To query whether a status word 0x9000 happened after SELECT command*/
}DCL_SIMDriver_t;


/* defines the possible application value*/
typedef enum
{
  SIM_CONFIG_AP_TYPE_PHONE1,    /*the card will be used for phone application SIM card 1*/
  SIM_CONFIG_AP_TYPE_PHONE2,    /*the card will be used for phone application SIM card 2*/
  SIM_CONFIG_AP_TYPE_CMMB_SMD,  /*the card will be used for CMMB SMD card*/
  SIM_CONFIG_AP_TYPE_PHONE3,    /*the card will be used for phone application SIM card 3*/
  SIM_CONFIG_AP_TYPE_PHONE4     /*the card will be used for phone application SIM card 4*/
} DCL_SIM_CONFIG_AP_TYPE;


/*************************************************************************
* FUNCTION
*  DclSim_Hook_Driver_Table
*
* DESCRIPTION
*     This function is to hook sim driver table, which is used for 3rd party's software SIM
*
* PARAMETERS
*    simIf - the sim interface which will be used as software SIM (Must be 0 in NBIOT project)
*    simDrv - a ponter of DCL_SIMDriver_t structure for software SIM, 
*                  can be NULL to restore hardware SIM driver
*
* RETURNS
*    a pointer of DCL_SIMDriver_t structure to orignal driver table
*
*************************************************************************/
extern DCL_SIMDriver_t *DclSim_Hook_Driver_Table(DCL_UINT32 simIf, DCL_SIMDriver_t *simDrv);

#endif //DCL_DEFINITION_PROTOTYPE

