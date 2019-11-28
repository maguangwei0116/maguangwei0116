
#include "rt_type.h"
#include "file.h"

#define URANDOM_DEV     "/dev/urandom"

uint64_t rt_get_random_num(void)
{
    uint64_t num;
    rt_fshandle_t fp = NULL;
    
    fp = linux_fopen(URANDOM_DEV, "r");
    if (fp) {
        linux_fseek(fp, 0, RT_FS_SEEK_SET);
        linux_fread((uint8_t *)&num, 1, sizeof(num), fp);        
        linux_fclose(fp);
    }

    return num;
}

