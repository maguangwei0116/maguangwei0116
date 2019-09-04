
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rt_type.h"

static void linux_get_random_buf(void *buf, uint32_t len)
{
#define URANDOM_DEV             "/dev/urandom"

    int ret;
    int fd = open(URANDOM_DEV, O_RDONLY);
    
    if (fd >= 0) {
        ret = read(fd, buf, len);
        close(fd);
    }

#undef URANDOM_DEV
}

uint64_t rt_get_random_num(void)
{
    uint64_t num;

    linux_get_random_buf(&num, sizeof(num)); 

    return num;
}

