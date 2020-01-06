
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : card_ext.h
 * Date        : 2019.12.24
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __CARD_EXT_H__
#define __CARD_EXT_H__

#include "rt_type.h"

#ifndef MAX_EID_LEN
#define MAX_EID_LEN         32
#endif
#ifndef MAX_ICCID_LEN
#define MAX_ICCID_LEN       20
#endif
#ifndef MAX_ICCID_NUM
#define MAX_ICCID_NUM       20
#endif
#ifndef MIN_PROFILES_LEN
#define MIN_PROFILES_LEN    1024
#endif

/**
 * @brief       get eUICc/vUICC EID 
 * @param       eid  - point the buffer that eid save to   
 * @param       size - buffer size of eid     
 * @return      0    - success   
 *              else - failure
 * @note        must: size > MAX_EID_LEN, no support mul-thread calling                    
 */
int32_t card_ext_get_eid(char *eid, int32_t size);

/**
 * @brief       get eUICc/vUICC profiles list 
 * @param[out]  profiles_info_json  - point the buffer that profiles save to. It will be JSON format string. 
 * @param[in]   size    - buffer size of profiles_info_json    
 * @return      0       - success   
                else    - failure, see error code in lpa
 * @note        size >= MIN_PROFILES_LEN, no support mul-thread calling
 *              sample data: {"profiles":[{"iccid":"89860317422045089815","type":1,"state":0},{"iccid":"89860317422045047482","type":2,"state":0},{"iccid":"89852019919070121569","type":2,"state":1}]}
 * @code
 *
 *  {
 *      #include "card_ext.h"
 *  
 *      char profiles_info_json[1024] = {0};
 *      int32_t ret = RT_ERROR;
 *  
 *      ret = card_ext_get_profiles_info(profiles_info_json, sizeof(profiles_info_json));
 *      if (!ret) {        
 *          // get profiles info ok
 *      } else {
 *          // get profiles info fail
 *      }
 *  }
 *
 * @endcode
 */
int32_t card_ext_get_profiles_info(char *profiles_info_json, int32_t size);

/**
 * @brief       delete a profile which specified by iccid
 * @param[in]   iccid - point the buffer that iccid (string format)      
 * @return      0     - success   
 *              else  - failure, see error code in lpa
 * @note        no support mul-thread calling 
 */
int32_t card_ext_delete_profile(const char *iccid);

/**
 * @brief       enable a profile which specified by iccid
 * @param[in]   iccid - point the buffer that iccid (string format)      
 * @return      0     - success   
 *              else  - failure, see error code in lpa
 * @note        no support mul-thread calling                             
 */
int32_t card_ext_enable_profile(const char *iccid);

/**
 * @brief       disable a profile which specified by iccid
 * @param[in]   iccid - point the buffer that iccid (string format)      
 * @return      0     - success   
 *              else  - failure, see error code in lpa
 * @note        no support mul-thread calling                             
 */
int32_t card_ext_disable_profile(const char *iccid);

#endif // __CARD_EXT_H__

