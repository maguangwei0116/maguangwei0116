#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include "rt_type.h"

#define RT_DATA_SIZE        32
#define RT_PING_TIMES       10

typedef struct tag_icmp_header
{
    uint8_t         type;
    uint8_t         code;
    uint16_t        check_sum;
    uint16_t        id;
    uint16_t        seq;
} icmp_header;

typedef struct tag_iphdr
{
    uint8_t         ip_head_verlen;
    uint8_t         ip_tos;
    uint16_t        ip_length;
    uint16_t        ip_id;
    uint16_t        ip_flags;
    uint8_t         ip_ttl;
    uint8_t         ip_protacol;
    uint16_t        ip_checksum;
    int32_t         ip_source;
    int32_t         ip_destination;
} ip_header;

uint16_t generation_checksum(uint16_t *buf, int32_t size)
{
    unsigned long cksum = 0;
    while(size > 1) {
        cksum += *buf++;
        size -= sizeof(uint16_t);
    }

    if(size) {
        cksum += *buf++;
    }

    cksum =  (cksum>>16) + (cksum & 0xffff);
    cksum += (cksum>>16);

    return (uint16_t)(~cksum);
}

double get_time_interval(struct timeval *start, struct timeval *end)
{
    double interval;
    struct timeval tp;

    tp.tv_sec = end->tv_sec - start->tv_sec;
    tp.tv_usec = end->tv_usec - start->tv_usec;

    if(tp.tv_usec < 0) {
        tp.tv_sec -= 1;
        tp.tv_usec += 1000000;
    }

    interval = tp.tv_sec * 1000 + tp.tv_usec * 0.001;
    return interval;
}

int32_t ping_host_ip(const uint8_t *domain, double *avg_delay, int32_t *lost, double *mdev)
{
    int32_t i;
    int32_t val = 1;
    int32_t ret = RT_ERROR;
    int32_t client_fd;
    int32_t size = 50 * 1024;
    int32_t recv_count = 0;
    double time_sum = 0;
    double time_interval[RT_PING_TIMES + 1] = {0};
    double time_mdev = 0;
    struct timeval timeout;
    uint8_t * icmp;
    in_addr_t dest_ip;
    icmp_header * icmp_head;
    struct sockaddr_in dest_socket_addr;

    MSG_PRINTF(LOG_DBG, "start ping...\n");

    if(domain == NULL) {
        MSG_PRINTF(LOG_ERR, "ping_host_ip domain is NULL !\n");
        return ret;
    }

    dest_ip = inet_addr(domain);
    if(dest_ip == INADDR_NONE) {
        struct hostent* p_hostent = gethostbyname(domain);
        if(p_hostent) {
            dest_ip = (*(in_addr_t*)p_hostent->h_addr);
        }
    }

    client_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (client_fd == RT_ERROR) {
        MSG_PRINTF(LOG_ERR, "socket error: %s !\n", strerror(errno));
        return ret;
    }

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(client_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    setsockopt(client_fd, IPPROTO_IP, IP_RECVERR, &val, sizeof(int32_t));       // icmp Port Unreachable

    if(setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval))) {
        MSG_PRINTF(LOG_ERR, "setsocketopt SO_RCVTIMEO error !\n");
        return ret;
    }

    if(setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval))) {
        MSG_PRINTF(LOG_ERR, "setsockopt SO_SNDTIMEO error !\n");
        return ret;
    }

    dest_socket_addr.sin_family = AF_INET;
    dest_socket_addr.sin_addr.s_addr = dest_ip;
    dest_socket_addr.sin_port = htons(0);
    memset(dest_socket_addr.sin_zero, 0, sizeof(dest_socket_addr.sin_zero));

    icmp = (uint8_t *)malloc(sizeof(icmp_header) + RT_DATA_SIZE);
    memset(icmp, 0, sizeof(icmp_header) + RT_DATA_SIZE);

    icmp_head = (icmp_header *)icmp;
    icmp_head->type = 8;
    icmp_head->code = 0;
    icmp_head->id = 1;

    MSG_PRINTF(LOG_DBG, "Ping %s, IP : %s\n", domain, inet_ntoa(*((struct in_addr*)&dest_ip)));

    for(i = 0; i < RT_PING_TIMES; i++) {
        struct timeval start;
        struct timeval end;
        long result;
        struct sockaddr_in from;
        socklen_t from_packet_len;
        long read_length;
        uint8_t recv_buf[1024];

        rt_os_sleep(1);

        icmp_head->seq = htons(i);
        icmp_head->check_sum = 0;
        icmp_head->check_sum = generation_checksum((uint16_t*)icmp,
            sizeof(icmp_header) + RT_DATA_SIZE);
        gettimeofday(&start, NULL);
        result = sendto(client_fd, icmp, sizeof(icmp_header) +
            RT_DATA_SIZE, 0, (struct sockaddr *)&dest_socket_addr,
            sizeof(dest_socket_addr));

        if(result == RT_ERROR) {
            // MSG_PRINTF(LOG_INFO, "time_sum is %lf\n", time_sum);
            MSG_PRINTF(LOG_DBG, "PING: sendto: Network is unreachable\n");
            continue;
        }

        from_packet_len = sizeof(from);
        memset(recv_buf, 0, sizeof(recv_buf));

        while(1) {
            read_length = recvfrom(client_fd, recv_buf, 1024, 0, (struct sockaddr*)&from, &from_packet_len);
            gettimeofday( &end, NULL );

            if(read_length != RT_ERROR) {
                ip_header * recv_ip_header = (ip_header*)recv_buf;
                int32_t ip_ttl = (int32_t)recv_ip_header->ip_ttl;
                icmp_header * recv_icmp_header = (icmp_header *)(recv_buf + (recv_ip_header->ip_head_verlen & 0x0F) * 4);

                if(recv_icmp_header->type != 0) {
                    MSG_PRINTF(LOG_ERR, "error type %d received, error code %d \n", recv_icmp_header->type, recv_icmp_header->code);
                    break;
                }

                if(recv_icmp_header->id != icmp_head->id) {
                    MSG_PRINTF(LOG_ERR, "some else's packet\n");
                    continue;
                }

                if(read_length >= (sizeof(ip_header) + sizeof(icmp_header) + RT_DATA_SIZE)) {
                    double tmp_time = get_time_interval(&start, &end);
                    recv_count ++;
                    time_sum += tmp_time;
                    time_interval[i] = tmp_time;
                    MSG_PRINTF(LOG_DBG, "time_interval[%d] is %lf\n", i, time_interval[i]);
                }

                break;
            } else {
                MSG_PRINTF(LOG_DBG, "receive data error !\n");
                break;
            }
        }
    }

PING_EXIT:
    if(NULL != icmp) {
        free(icmp);
        icmp = NULL;
    }

    if(client_fd != RT_ERROR) {
        close(client_fd);
    }

    for (i = 0; i < recv_count; i++) {
        if (time_sum/recv_count > time_interval[i]) {
            time_mdev += time_sum/recv_count - time_interval[i];
        } else {
            time_mdev += time_interval[i] - time_sum/recv_count;
        }
    }

    *lost = RT_PING_TIMES - recv_count;
    *avg_delay = time_sum/recv_count;
    *mdev = time_mdev/recv_count;

    return ret;
}

int32_t local_ping(int32_t argc, uint8_t *argv[], double *avg_delay, int32_t *lost, double *mdev)
{
    ping_host_ip(argv[3], avg_delay, lost, mdev);
}

int32_t rt_local_ping(uint8_t *ip, double *delay, int32_t *lost, double *mdev)
{
    ping_host_ip(ip, delay, lost, mdev);
}
