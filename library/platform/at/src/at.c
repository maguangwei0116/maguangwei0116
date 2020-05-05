
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>

#include "rt_os.h"
#include "rt_type.h"
#include "log.h"

#define SYS_AT_PORT_NAME        "rt_at_port"
#define DEFAULT_AT_TIME_OUT     3000
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)           sizeof(a)/sizeof(a[0])
#endif

static char g_at_port_name[32];
static rt_pthread_mutex_t *g_at_mutex;

static int32_t at_set_port_name(const char *at_port)
{
    if (at_port && rt_os_strlen(at_port)) {
        snprintf(g_at_port_name, sizeof(g_at_port_name), "%s", at_port);
    }

    return RT_SUCCESS;
}

int32_t at_send_recv(const char *cmd, char *rsp, int32_t rsp_len, int32_t timeout_ms)
{
    int32_t ret;
    int32_t len;
    int32_t recv_len;
    fd_set fds;
    int32_t fd = -1;
    struct timeval timeout = {0};

    rt_os_memset(rsp, 0, rsp_len);
    linux_mutex_lock(g_at_mutex);
    
    fd = open(g_at_port_name, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (fd < 0) {   // Try again
        fd = open(g_at_port_name, O_RDWR | O_NONBLOCK | O_NOCTTY);
        if (fd < 0) {
            MSG_PRINTF(LOG_ERR, "Open AT Port %s fail !\n", g_at_port_name);
            ret = RT_ERROR;
            goto exit;
        }
    }
    //MSG_PRINTF(LOG_INFO, "Open AT Port[%d]: %s\n", fd, g_at_port_name);

    timeout.tv_sec  = timeout_ms / 1000;
    timeout.tv_usec = timeout_ms % 1000;

    len = rt_os_strlen(cmd);
    ret = write(fd, cmd, len);
    if (ret != len) {
        MSG_PRINTF(LOG_ERR, "write at cmd fail !\n");
        ret = RT_ERROR;
        goto exit;
    }
    //MSG_PRINTF(LOG_INFO, "Send AT CMD [%d]: %s \n", len, cmd);

    len = 0;
    while (1) {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        switch (select(fd+1, &fds, NULL, NULL, &timeout)) {
            case -1:
                MSG_PRINTF(LOG_WARN, "Select error\n");
                ret = RT_ERROR;
                goto exit;

            case 0:
                MSG_PRINTF(LOG_WARN, "Time out\n");
                ret = RT_ERROR;
                goto exit;

            default:
                /* A AT rsp may rsp with 2 data sections */
                if (FD_ISSET(fd, &fds)) {
                    recv_len = read(fd, &rsp[len], rsp_len-len);
                    //MSG_HEXDUMP("at-rsp", &rsp[len], recv_len);                    
                    if (rt_os_strstr(rsp, "OK") || rt_os_strstr(rsp, "ERROR")) {
                        len += recv_len;
                        //MSG_PRINTF(LOG_INFO, "Read AT RSP [%d]: %s\n", len, rsp);
                        ret = RT_SUCCESS;
                        goto exit;
                    } else if (recv_len > 0) {
                        len += recv_len;
                        continue;
                    } else {
                        MSG_PRINTF(LOG_WARN, "Read at rsp fail\n");
                        ret = RT_ERROR;
                        goto exit;
                    }
                    
                } else {
                    MSG_PRINTF(LOG_WARN, "FD is missed\n");
                }
                break;
        }
    }

exit:
    
    linux_mutex_unlock(g_at_mutex);
    if (fd > 0) {
        close(fd);
    }

    return ret;
}

static int32_t at_send_inner(const char *cmd, char *rsp, int32_t rsp_len)
{
    return at_send_recv(cmd, rsp, rsp_len, DEFAULT_AT_TIME_OUT);
}

int32_t init_at(void *arg)
{
    /* Add your possible AT port name here */
    const char *at_port_name_list[] = 
    {
        "/dev/smd8",    // EC20
        "/dev/smd9",    // EC25 
    };
    const char *at_cmd = "AT\r\n";
    char at_rsp[1024] = {0};
    int32_t i;
    int32_t ret;
    int32_t cnt = ARRAY_SIZE(at_port_name_list);

    (void)arg;
    g_at_mutex = linux_mutex_init();

    if (!linux_rt_file_exist(SYS_AT_PORT_NAME)) {
        rt_create_file(SYS_AT_PORT_NAME);
        /* try to get terminal actual AT port name */
        for (i = 0; i < cnt; i++) {
            at_set_port_name(at_port_name_list[i]);
            ret = at_send_inner(at_cmd, at_rsp, sizeof(at_rsp));
            if (!ret) {
                MSG_PRINTF(LOG_DBG, "AT Port: %s\n", g_at_port_name);
                rt_write_data(SYS_AT_PORT_NAME, 0, g_at_port_name, sizeof(g_at_port_name));
                break;
            }
        }
        if (i == cnt) {
            MSG_PRINTF(LOG_WARN, "find AT Port fail\n");
            ret = RT_ERROR;
        }
    } else {
        ret = rt_read_data(SYS_AT_PORT_NAME, 0, g_at_port_name, sizeof(g_at_port_name));
        MSG_PRINTF(LOG_DBG, "AT Port: %s, ret=%d\n", g_at_port_name, ret);
    }

    return ret;
}
