/*****************************************************************************
Filename: callback.c
Author  :
Date    : 2019-10-16 10:07:22
Description:
*****************************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "cb_io.h"

#define  SERVER_PATH                   "/data/cos_server"
#define  MAX_SOCKFD_NUM                 3
#define  IO_INTERFACE_INVALID           0xFF
#define  IO_INTERFACE_MODEM             0
#define  IO_INTERFACE_LPA               1
#define  IO_MAX_INTERFACES              2

// TODO: add 2625/9205/unisoc 9832e etc.

fd_set io_fd;
int sock_fd[MAX_SOCKFD_NUM] = {-1, -1, -1};  // 0；listen，1、2:message

static int find_max_sock_fd(void)
{
    int max_fd = -1;
    int i;

    for (i = 0; i < MAX_SOCKFD_NUM; i++) {
        if (sock_fd[i] > max_fd) {
            max_fd = sock_fd[i];
        }
    }
    return max_fd;
}


result_t linux_io_server_init(void)
{
    int ret = 1;
    struct sockaddr_un server_sai;
    struct sigaction sa;
    int flags;

    if (sock_fd[0] < 0) {
        sock_fd[0] = sock_fd[1] = sock_fd[2] = -1;  // Initialize the sock_fd

        sock_fd[0] = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock_fd[0] < 0) {
            printf("connect create communication socket\n");
            _exit(0);
        }

        flags = fcntl(sock_fd[0], F_GETFL, 0);
        fcntl(sock_fd[0], F_SETFL, flags | O_NONBLOCK);

        memset(&server_sai, 0, sizeof(server_sai));
        server_sai.sun_family = AF_UNIX;
        server_sai.sun_path[0] = '\0';  // must be '\0'
        strncpy(server_sai.sun_path, SERVER_PATH, sizeof(server_sai.sun_path) - 1);

        ret = bind(sock_fd[0], (struct sockaddr *)&server_sai, sizeof(struct sockaddr_un));
        if (ret < 0) {
            printf("socket %d bind false result:%s\n", sock_fd[0], strerror(errno));
            close(sock_fd[0]);
            unlink(SERVER_PATH);
            _exit(0);
        }

        ret = listen(sock_fd[0], IO_MAX_INTERFACES);
        if (ret < 0) {
            printf("socket %d listen false result:%s\n", sock_fd[0], strerror(errno));
            close(sock_fd[0]);
            unlink(SERVER_PATH);
            _exit(0);
        }

        sa.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &sa, 0);
        printf("io start [ok]\n");
    }else {
        printf("io start [ok]\n");
    }
    return 0;
}

result_t linux_io_server_wait(uint32_t timeout, io_id_t *io_id)
{
    int i;

select_loop:
    FD_ZERO(&io_fd);
    FD_SET(sock_fd[0], &io_fd);

    if ((*io_id == IO_INTERFACE_MODEM) && (sock_fd[IO_INTERFACE_MODEM + 1] > 0)) {
        FD_SET(sock_fd[IO_INTERFACE_MODEM + 1], &io_fd);
        i = sock_fd[IO_INTERFACE_MODEM + 1];
    } else if ((*io_id == IO_INTERFACE_LPA) && (sock_fd[IO_INTERFACE_LPA + 1] > 0)) {
        FD_SET(sock_fd[IO_INTERFACE_LPA + 1], &io_fd);
        i = sock_fd[IO_INTERFACE_LPA + 1];
    } else if (*io_id == IO_INTERFACE_INVALID) {
        if (sock_fd[IO_INTERFACE_LPA + 1] >= 0) {
            FD_SET(sock_fd[IO_INTERFACE_LPA + 1], &io_fd);
        }

        if (sock_fd[IO_INTERFACE_MODEM + 1] >= 0) {
            FD_SET(sock_fd[IO_INTERFACE_MODEM + 1], &io_fd);
        }
        i = find_max_sock_fd();
    } else {
        printf("Wrong io_id\n");
        return -1;
    }

    if(select(i + 1, &io_fd, NULL, NULL, NULL) > 0) {
        if (FD_ISSET(sock_fd[0], &io_fd)) {
            struct sockaddr_in client_sai;
            int addrlen = sizeof(struct sockaddr);
            int connect_id = accept(sock_fd[0], (struct sockaddr *)&client_sai, (socklen_t *)&addrlen);

            if (connect_id > 0) {
                if (sock_fd[1] < 0) {
                    sock_fd[1] = connect_id;
                } else if (sock_fd[2] < 0) {
                    sock_fd[2] = connect_id;
                }
            }
        #if (IO_DATA_DEBUG == ON)
            printf("io accept client connect result: %d\n", connect_id);
        #endif
            goto select_loop;
        }

        i = 0;
        if (sock_fd[1] >=0 && FD_ISSET(sock_fd[1], &io_fd)) {
            // TODO:判断合法性,私有协议
            *io_id = IO_INTERFACE_MODEM;
        #if (IO_DATA_DEBUG == ON)
            printf("io:%d have some data wait to recv\n", sock_fd[1]);
        #endif
            return 0;
        }

        if (sock_fd[2] >=0 && FD_ISSET(sock_fd[2], &io_fd)) {
            // TODO:判断合法性，私有协议
            *io_id = IO_INTERFACE_LPA;
        #if (IO_DATA_DEBUG == ON)
            printf("io:%d have some data wait to recv\n", sock_fd[2]);
        #endif
            return 0;
        }
    }
    return 0;
}

int32_t linux_io_server_send(io_id_t io_id, uint8_t *rsp, uint16_t len)
{
    result_t ret = send(sock_fd[io_id + 1], rsp, len, 0);
    if (ret <= 0) {
        if (sock_fd[io_id + 1] > 0) {
            printf("Close %d socket\n", sock_fd[io_id + 1]);
            close(sock_fd[io_id + 1]);
        }
        sock_fd[io_id + 1] = -1;
    }
    return ret;
}

int32_t linux_io_server_recv(io_id_t io_id, uint8_t *rsq, uint16_t len)
{
    result_t ret = recv(sock_fd[io_id + 1], rsq, len, 0);
    if (ret <= 0) {
        if (sock_fd[io_id + 1] > 0) {
            printf("Close %d socket\n", sock_fd[io_id + 1]);
            close(sock_fd[io_id + 1]);
        }
        sock_fd[io_id + 1] = -1;
    }
    return ret;
}

int connect_fd = -1;

result_t linux_io_client_connect(void)
{
    struct sockaddr_un srv_addr;
    connect_fd = socket(PF_UNIX,SOCK_STREAM, 0);
    if(connect_fd < 0){
        printf("cannot creat socket\n");
        return -1;
    }
    srv_addr.sun_family=AF_UNIX;
    strcpy(srv_addr.sun_path, SERVER_PATH);
    if (connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0) {
        printf("cannot connect server\n");
        close(connect_fd);
        return -1;
    }
    return 0;
}

result_t linux_io_client_close(void)
{
    close(connect_fd);
    connect_fd = -1;
    return 0;
}

int32_t linux_io_client_send(uint8_t *rsq, uint16_t len)
{
    return send(connect_fd, rsq, len, 0);
}

int32_t linux_io_client_recv(uint8_t *rsp, uint16_t len)
{
    int ret = 0, recv_len = len;
    while(len != 0) {
        ret = recv(connect_fd, rsp, len, 0);
        if(ret < 0) {
            if((errno == EINTR) || (errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                printf("\nsocket recv failed: %s, continue\n", strerror(errno));
                continue;
            }
            else {
                printf("\nsocket recv failed: %s\n", strerror(errno));
                close(connect_fd);
                return -1;
            }
        }
        else if(ret == 0) {
            printf("\nsocket closed: %s\n", strerror(errno));
            close(connect_fd);
            return -1;
        }
        len -= ret;
        rsp += ret;
    } // end while
    return recv_len;
}
