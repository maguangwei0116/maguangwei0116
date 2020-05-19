
#include <dlfcn.h>

#include "rt_type.h"
#include "log.h"
#include "ubi.h"

/* config ubi Manufacturers */
#define QUECTEL_9X07_UBI_ENABLE             1

#define USR_LIB_PATH                        "/usr/lib/"
#define INIT_OBJ(module, so, api, func)     {#module, #so, #api, func}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)                       (sizeof((a)) / sizeof((a)[0]))
#endif

#ifdef QUECTEL_9X07_UBI_ENABLE
/** 
 * Upgrade oemapp partition result
 *   
 */
typedef enum {	
    QL_SOFTSIM_UPDATE_OK = 0,	
    QL_SOFTSIM_UPDATE_FAILED,	
    QL_SOFTSIM_UPDATE_INVALID
} _QL_SOFTSIM_RETURN_TYPE_;

/** 
 * Upgrade oemapp partition 
 * 
 * @param [in] file             the source oemapp ubi file 
 * 
 * @return  
 *   On success, 0 is returned.  On error, -1 is returned. 
 *  
 */
extern _QL_SOFTSIM_RETURN_TYPE_ ql_file_update(char *file);

static int32_t ubi_update_quectel(void *func, const char *ubi_file)
{
    ubi_update_func update_func = (ubi_update_func)func;

    MSG_PRINTF(LOG_TRACE, "%s, func %p, %s !\r\n", __func__, func, ubi_file); 

    return update_func(ubi_file) == QL_SOFTSIM_UPDATE_OK ? UBI_UPDATE_OK : UBI_UPDATE_FAIL;
}

#endif

static ubi_update_api_t g_ubi_update_api_list[] = 
{
#ifdef QUECTEL_9X07_UBI_ENABLE
    INIT_OBJ(QUECTEL_9x07,  libql_softsim_extsdk.so.0,  ql_file_update,     ubi_update_quectel),
#endif
};

int32_t ubi_update(const char *ubi_file)
{
    void *handle = NULL;
    char *error = NULL;
    char library[128];
    const ubi_update_api_t *obj = NULL;
    void *func = NULL;
    int32_t i;
    int32_t ret = UBI_UPDATE_FAIL;
    
    for (i = 0; i < ARRAY_SIZE(g_ubi_update_api_list); i++) {
        obj = &g_ubi_update_api_list[i];
        //MSG_PRINTF(LOG_INFO, "%s, %s, %s\r\n", obj->module, obj->so_name, obj->api_name);  
        snprintf(library, sizeof(library), "%s/%s", USR_LIB_PATH, obj->so_name);
        handle = dlopen(library, RTLD_LAZY);
        if (!handle) {
            MSG_PRINTF(LOG_ERR, "dlopen %s fail, %s !\r\n", library, dlerror());
            continue;
        } else {
            dlerror();
            func = dlsym(handle, obj->api_name);  
            if ((error = dlerror()) != NULL)  {  
                MSG_PRINTF(LOG_ERR, "dlsym %s fail, %s !\r\n", library, error);
                continue; 
            } 

            if (obj->func && func) {
                ret = obj->func(func, ubi_file);
                goto exit_entry;
            }

            dlclose(handle);
        }
    }

exit_entry:

    if (handle) {
        dlclose(handle);
        handle = NULL;
    }

    return ret;
}
