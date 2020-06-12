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

#define DATA_SIZE 32

typedef struct tag_icmp_header
{
    u_int8_t  type;
    u_int8_t  code;
    u_int16_t check_sum;
    u_int16_t id;
    u_int16_t seq;
} icmp_header;

typedef struct tag_iphdr
{
    u_int8_t        ip_head_verlen;
    u_int8_t        ip_tos;
    unsigned short  ip_length;
    unsigned short  ip_id;
    unsigned short  ip_flags;
    u_int8_t        ip_ttl;
    u_int8_t        ip_protacol;
    unsigned short  ip_checksum;
    int             ip_source;
    int             ip_destination;
} ip_header;

unsigned short generation_checksum(unsigned short * buf, int size)
{
    unsigned long cksum = 0;
    while(size > 1)
    {
        cksum += *buf++;
        size -= sizeof(unsigned short);
    }

    if(size)
    {
        cksum += *buf++;
    }

    cksum =  (cksum>>16) + (cksum & 0xffff);
    cksum += (cksum>>16);

    return (unsigned short)(~cksum);
}

double get_time_interval(struct timeval * start, struct timeval * end)
{
    double interval;
    struct timeval tp;

    tp.tv_sec = end->tv_sec - start->tv_sec;
    tp.tv_usec = end->tv_usec - start->tv_usec;
    if(tp.tv_usec < 0)
    {
        tp.tv_sec -= 1;
        tp.tv_usec += 1000000;
    }

    interval = tp.tv_sec * 1000 + tp.tv_usec * 0.001;
    return interval;
}

int ping_host_ip(const char * domain, double *avg_delay, int *lost, double *mdev)
{
    int i;
    int ret = -1;
    int client_fd;
    int size = 50 * 1024;
    int recv_count = 0;
    double time_sum = 0;
    double time_interval[10] = {0};
    double time_mdev = 0;
    struct timeval timeout;
    char * icmp;
    in_addr_t dest_ip;
    icmp_header * icmp_head;
    struct sockaddr_in dest_socket_addr;

    if(domain == NULL)
    {
        MSG_PRINTF(LOG_ERR, "ping_host_ip domain is NULL !\n");
        return ret;
    }

    dest_ip = inet_addr(domain);
    if(dest_ip == INADDR_NONE)
    {
        struct hostent* p_hostent = gethostbyname(domain);
        if(p_hostent)
        {
            dest_ip = (*(in_addr_t*)p_hostent->h_addr);
        }
    }

    client_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (client_fd == -1)
    {
        MSG_PRINTF(LOG_ERR, "socket error: %s !\n", strerror(errno));
        return ret;
    }

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(client_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    if(setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)))
    {
        MSG_PRINTF(LOG_ERR, "setsocketopt SO_RCVTIMEO error !\n");
        return ret;
    }

    if(setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval)))
    {
        MSG_PRINTF(LOG_ERR, "setsockopt SO_SNDTIMEO error !\n");
        return ret;
    }

    dest_socket_addr.sin_family = AF_INET;
    dest_socket_addr.sin_addr.s_addr = dest_ip;
    dest_socket_addr.sin_port = htons(0);
    memset(dest_socket_addr.sin_zero, 0, sizeof(dest_socket_addr.sin_zero));

    icmp = (char *)malloc(sizeof(icmp_header) + DATA_SIZE);
    memset(icmp, 0, sizeof(icmp_header) + DATA_SIZE);

    icmp_head = (icmp_header *)icmp;
    icmp_head->type = 8;
    icmp_head->code = 0;
    icmp_head->id = 1;

    // MSG_PRINTF(LOG_DBG, "PING %s (%s)\n", domain, inet_ntoa(*((struct in_addr*)&dest_ip)));

    for(i = 0; i < 10; i++)
    {
        sleep(1);
        struct timeval start;
        struct timeval end;
        long result;
        struct sockaddr_in from;
        socklen_t from_packet_len;
        long read_length;
        char recv_buf[1024];

        icmp_head->seq = htons(i);
        icmp_head->check_sum = 0;
        icmp_head->check_sum = generation_checksum((unsigned short*)icmp,
            sizeof(icmp_header) + DATA_SIZE);
        gettimeofday(&start, NULL);
        result = sendto(client_fd, icmp, sizeof(icmp_header) +
            DATA_SIZE, 0, (struct sockaddr *)&dest_socket_addr,
            sizeof(dest_socket_addr));
        if(result == -1)
        {
            time_sum += 9999.99;
            time_interval[i] = 9999.99;
            MSG_PRINTF(LOG_INFO, "time_sum is %lf\n", time_sum);
            MSG_PRINTF(LOG_DBG, "PING: sendto: Network is unreachable\n");

            continue;
        }

        from_packet_len = sizeof(from);
        memset(recv_buf, 0, sizeof(recv_buf));
        while(1)
        {
            read_length = recvfrom(client_fd, recv_buf, 1024, 0,
                (struct sockaddr*)&from, &from_packet_len);
            gettimeofday( &end, NULL );

            if(read_length != -1)
            {
                ip_header * recv_ip_header = (ip_header*)recv_buf;
                int ip_ttl = (int)recv_ip_header->ip_ttl;
                icmp_header * recv_icmp_header = (icmp_header *)(recv_buf +
                    (recv_ip_header->ip_head_verlen & 0x0F) * 4);

                if(recv_icmp_header->type != 0)
                {
                    MSG_PRINTF(LOG_ERR, "error type %d received, error code %d \n", recv_icmp_header->type, recv_icmp_header->code);
                    break;
                }

                if(recv_icmp_header->id != icmp_head->id)
                {
                    MSG_PRINTF(LOG_ERR, "some else's packet\n");
                    continue;
                }

                if(read_length >= (sizeof(ip_header) +
                    sizeof(icmp_header) + DATA_SIZE))
                {
                    double tmp_time = get_time_interval(&start, &end);
                    // MSG_PRINTF(LOG_DBG, "%ld bytes from %s (%s): icmp_seq=%d ttl=%d time=%.2f ms\n",
                    //     read_length, domain, inet_ntoa(from.sin_addr), recv_icmp_header->seq / 256,
                    //     ip_ttl, tmp_time);

                    ret = 0;
                    recv_count ++;
                    time_sum += tmp_time;
                    time_interval[i] = tmp_time;

                    // MSG_PRINTF(LOG_INFO, "time_sum is %lf\n", time_sum);
                    // MSG_PRINTF(LOG_INFO, "time_interval[%d] is %lf\n", i, time_interval[i]);
                }

                break;
            }
            else
            {
                MSG_PRINTF(LOG_ERR, "receive data error !\n");
                time_sum += 9999.99;
                time_interval[i] = 9999.99;
                break;
            }
        }
    }

PING_EXIT:
    if(NULL != icmp)
    {
        free(icmp);
        icmp = NULL;
    }

    if(client_fd != -1)
    {
        close(client_fd);
    }

    for (i = 0; i < 10; ++i) {
        if (time_sum / 10 > time_interval[i]) {
            time_mdev += time_sum / 10 - time_interval[i];
        } else {
            time_mdev += time_interval[i] - time_sum / 10;
        }
    }

    // MSG_PRINTF(LOG_INFO, "lost is %d\n", (10 - recv_count) * 10);
    // MSG_PRINTF(LOG_INFO, "time_sum / 10 is %lf\n", time_sum / 10);
    // MSG_PRINTF(LOG_INFO, "time_mdev / 10 is %lf\n", time_mdev / 10);

    *lost = (10 - recv_count) * 10;
    *avg_delay = time_sum / 10;
    *mdev = time_mdev / 10;

    return ret;
}

int local_ping(int argc, char *argv[], double *avg_delay, int *lost, double *mdev)
{
    ping_host_ip(argv[3], avg_delay, lost, mdev);
}

int rt_local_ping(char *ip, double *delay, int *lost, double *shake)
{
    ping_host_ip(ip, delay, lost, shake);
}
