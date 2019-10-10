
#include "rt_type.h"
#include "rt_manage_data.h"

#define URANDOM_DEV             "/dev/urandom"

uint64_t rt_get_random_num(void)
{
    uint64_t num;

    rt_read_data(URANDOM_DEV, 0, (uint8_t *)&num, sizeof(num));

    return num;
}

