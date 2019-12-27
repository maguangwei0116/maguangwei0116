
#ifdef CFG_USR_DNS_API

#include "rt_type.h"
#include "log.h"
#include "dns.h"

#include <stdio.h>  

///////////////////////////////////////////////////////////////////////////////////////////////////

//#define PLATFORM_WINDOWS              1  //��ʾwindowsƽ̨
#define PLATFORM_LINUX                  1  //��ʾlinuxƽ̨

#define DNS_SERVER_PORT                 53
#define MAX_IP_LEN                      16 // only for IPV4
#define MAX_IP_CNT                      1
#define MAX_TRY_CNT                     1
#define DNS_TIMEOUTS                    5  // 5 seconds
#define DNS_PACKET_ID                   0x55AA
#define DNS_DEBUG_ENABLE                0  // debug enable

#if (DNS_DEBUG_ENABLE)
#define DNS_PRINTF(fmt, arg...)         MSG_PRINTF(LOG_INFO, fmt, ##arg)
#define DNS_ORG_PRINTF(fmt, arg...)     MSG_ORG_PRINTF(LOG_INFO, fmt, ##arg)
#define DNS_HEXDUMP(title, buf, len)    MSG_INFO_ARRAY(title, buf, len)
#else
#define DNS_PRINTF(fmt, arg...)         do {} while(0)
#define DNS_ORG_PRINTF(fmt, arg...)     do {} while(0)
#define DNS_HEXDUMP(title, buf, len)    do {} while(0)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)                   sizeof(a)/sizeof(a[0])
#endif

/* 
Add your dns server here, but considering the IP white list !
*/
static const char *g_dns_server_list[] = 
{
    "114.114.114.114",  // 114 DNS   
    "8.8.8.8",          // Google DNS
    "208.67.222.222",   // OpenDNS
};

///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined (PLATFORM_LINUX)
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
//#include "debug.h"
#endif

#include "dns.h"

#if defined (PLATFORM_WINDOWS)
#include <winsock2.h> 
#endif

#if defined (PLATFORM_WINDOWS)
#pragma comment(lib,"ws2_32.lib")  
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

/* ������ѯ������ͷ���壬1�ֽڶ���*/
#if defined(PLATFORM_WINDOWS)
#pragma  pack(push)
#pragma  pack(1)
#endif

struct DnsHeader {
    unsigned short id;

    unsigned char rd:1;         /* �����ݹ����*/
    unsigned char tc:1;         /* ����δ�ض� */
    unsigned char aa:1;         /* ��Ȩ���������� */
    unsigned char opcode:4;     /* ��׼��ѯ */
    unsigned char qr:1;         /* 0:��ѯ,1:��Ӧ */

    unsigned char rcode:4;      /* ��Ӧ��:0û�г��� */
    unsigned char z:3;          /* ��������ʹ�� */
    unsigned char ra:1;         /* DNS�������Ƿ�֧�ֵݹ���� */

    unsigned short qdCount;     /* ������*/
    unsigned short anCount;     /* Ӧ����*/
    unsigned short nsCount;     /* ��Ȩ������ */
    unsigned short arCount;     /* ������Ϣ�� */
} 
#if defined (PLATFORM_LINUX)
__attribute__((packed))
#endif
;

#if defined(PLATFORM_WINDOWS)
#pragma  pack(pop)
#endif

#define COMMENT_MAX 64
#define SYMBOL_MAX  8

struct QueryType
{
    unsigned short q_class;
    unsigned char mnemonic_symbol[SYMBOL_MAX];
    unsigned char comment[COMMENT_MAX];
};


static unsigned short packet_id = DNS_PACKET_ID;

///////////////////////////////////////////////////////////////////////////////////////////////////

/* �����ֽ��� */
static inline void exchangeByteOrder(unsigned char *byte)
{
    unsigned char temp;

    temp = byte[0];
    byte[0] = byte[1];
    byte[1] = temp;
}

/* ����һ���ֵ��ֽ�˳�� */
static inline void exchangeWordOrder(unsigned int *word)
{
    unsigned char temp;
    unsigned char *ptr[4];

    ptr[0] = (unsigned char *)(word);
    ptr[1] = ((unsigned char *)(word))+1;
    ptr[2] = ((unsigned char *)(word))+2;
    ptr[3] = ((unsigned char *)(word))+3;

    temp = *ptr[0];
    *ptr[0] = *ptr[3];
    *ptr[3] = temp;

    temp = *ptr[1];
    *ptr[1] = *ptr[2];
    *ptr[2] = temp;
}

/* ���� DNS �����İ�ͷ(12 bytes) */
static int dns_packet_hdr_construct(struct DnsHeader *header)
{
    /* ����DNS��ѯ��������ͷ,û��ʹ�õ��ֶα�������Ϊ0 */
    header->id = packet_id;     /* ID ,�ɽ�������ָ����������ʶ */
    header->rd = 1;             /*�����ݹ����*/
    header->tc = 0;
    header->aa = 0;
    header->opcode = 0;         /*��׼��ѯ */
    header->qr = 0;
    header->rcode = 0;
    header->z = 0;
    header->ra = 0;
    header->qdCount = 1;        /*�������� */
    header->anCount = 0;
    header->nsCount = 0;
    header->arCount = 0;

    /* 16 bits �ı��ظ�ʽת��Ϊ�����ʽ */
    exchangeByteOrder((unsigned char *)&header->anCount);
    exchangeByteOrder((unsigned char *)&header->qdCount);
    exchangeByteOrder((unsigned char *)&header->nsCount);
    exchangeByteOrder((unsigned char *)&header->arCount);

    return sizeof(struct DnsHeader);
}

/* ���� NDS ���������� */
static int dns_packet_body_construct(char *packet_body, const char *buf)
{
    const char *domain_ptr = buf ;
    int i = 0, j = 0;

    while(*domain_ptr != '\0') {
        if(*domain_ptr != '.') {
            packet_body[i+1] = *domain_ptr;
            j++;
        } else {
            packet_body[i-j] = j;
            j = 0;
        }
        i++; domain_ptr++;
    }
    packet_body[i-j] = j;

    packet_body[++i] = 0;           /* null */
    packet_body[++i] = 0x00;
    packet_body[++i] = 0x01;        /* ��ѯ����  Ϊ 1 �� IPv4��ַ */

    packet_body[++i] = 0x00;        /* ��ѯ�� Ϊ 1 : Inetnet���� */
    packet_body[++i] = 0x01;

    return i;
}

/**
 * ���� ����
 * packet:���ݰ�ͷ��ָ��
 * body:  ָ��������ʼ����
 */
static const char *dns_packet_question_resovle(const struct DnsHeader *packet, const char *body)
{
    const char *ptr;
    unsigned char cnt = 0;
    unsigned char len = 0;

    (void)len;
    DNS_PRINTF("the queston: \n");
    DNS_PRINTF("Name : ");

    ptr = body;
    while(*ptr != '\0') {
        cnt = *ptr++;
        while(cnt--) {
            DNS_ORG_PRINTF("%c", *ptr);
            ptr++;
        }
        if(*ptr != '\0') {
            DNS_ORG_PRINTF(".");
        }
    }
    DNS_ORG_PRINTF("\n");

    ptr++;  /* skip null */
    DNS_PRINTF("Type : ");
    if(*ptr == 0x0 && *(ptr+1) == 0x1) {
        DNS_ORG_PRINTF("Host Address\n");
    }else{
        DNS_ORG_PRINTF("%02x%02x\n",*ptr, *(ptr+1));
    }

    ptr += 2; /* skip Type area */
    DNS_PRINTF("Class : ");
    if(*ptr == 0x0 && *(ptr+1) == 0x01) {
        DNS_ORG_PRINTF("IN\n");
    } else {
        DNS_ORG_PRINTF("%02x%02x\n", *ptr, *(ptr+1));
    }

    ptr += 2;  /* skip Class area */

    return ptr;
}

/* �������� */
static const char *dns_packet_name_resovle(const struct DnsHeader *packet, const  char *name)
{
    const char *p;
    unsigned char cnt,len;

    p = name;
    len = 0;

    while(*p != '\0') {
        if(((*p & 0xC0) >> 6) == 0x3) {
            p = (const char *)packet + (((*p & 0x3F)<<8) | *(p+1)); 
            p = dns_packet_name_resovle(packet, p);
        } else {
            cnt = *p++;
            while(cnt--) {
                DNS_ORG_PRINTF("%c",*p);
                p++;
            }

            if(*p != '\0') {
                DNS_ORG_PRINTF(".");
            }
        }
    }

    return  p;
}

/* ������Դ���� */
static const char *dns_packet_rr_resovle(const struct DnsHeader *packet, const  char *rr, 
                                    unsigned short rlen, char *ipout, int *iplen)
{
    const char *p;
    unsigned char cnt = 0;

    if (*iplen) { /* *clear ip length */
        *iplen = -1;
    }

    if(rlen == 0x04) {        /* IP addr */
        DNS_ORG_PRINTF("%d.%d.%d.%d\n", (unsigned char)rr[0],(unsigned char)rr[1],
            (unsigned char)rr[2],(unsigned char)rr[3]);
        if (ipout && iplen) {
            snprintf(ipout, *iplen, "%d.%d.%d.%d", (unsigned char)rr[0],(unsigned char)rr[1],
                                        (unsigned char)rr[2],(unsigned char)rr[3]);
            *iplen = strlen(ipout);
        }
        return (rr += 4);
    }

    while(rlen > 0) {
        if(((*rr & 0xC0) >> 6) == 0x03) {
            p = (const char *)packet + ((*rr & 0x3F)<<8 | (*(rr+1)));
            rlen -= 2;
            cnt = *p++;
            rr +=  2;
        } else {
            p = rr;
            cnt = *p++;
            rlen -= ( cnt + 1 );
            rr += ( cnt + 1 );
        }

        while(cnt--) {
            DNS_ORG_PRINTF("%c", *p);
            p++;
        }

        if(rlen != 0) {
            DNS_ORG_PRINTF(".");
        }
    }

    DNS_ORG_PRINTF("\n");

    return rr;
}

/**
 * ���� Ӧ��
 * packet:���ݰ�ͷ��ָ��
 * body:  ָ��Ӧ����ʼ����
 */
static const char *dns_packet_response_resovle(const struct DnsHeader *packet, const char *body, 
                                                    char *ipout, int *iplen)
{
    const char *ptr;
    unsigned char cnt = 0;
    unsigned char len = 0;
    unsigned int  ttl = 0;
    unsigned short rs_len = 0;

    (void)cnt;
    (void)len;

    DNS_PRINTF("the answer:\n");
    DNS_PRINTF("Name : ");
    
    ptr = dns_packet_name_resovle(packet, body);

    DNS_ORG_PRINTF("%s\n", ptr);

    ptr = body + 2;  /* skip null ,Get Type (2 bytes) */
    DNS_PRINTF("Type : ");
    if((*ptr == 0x00) && (*(ptr+1) == 0x01)) {
        DNS_ORG_PRINTF("Host Address\n");
    } else if ((*ptr == 0x00) && (*(ptr+1) == 0x05)) {
        DNS_ORG_PRINTF("CNAME (Canonical name for an alias)\n");
    } else {
        DNS_ORG_PRINTF("%02x%02x\n",*ptr, *(ptr+1));
    }

    ptr += 2; /* skip Type area , Get Class (2 bytes) */
    DNS_PRINTF("Class : ");
    if(*ptr == 0x00 && *(ptr+1) == 0x01) {
        DNS_ORG_PRINTF("IN\n");
    } else {
        DNS_ORG_PRINTF("%02x%02x\n", *ptr, *(ptr+1));
    }

    ptr += 2;  /* skip Class area ,Get TTL(4 bytes) */
    ttl = *(unsigned int *)(ptr);
    exchangeWordOrder(&ttl);

    DNS_PRINTF("TTL(Time To live): +++++++%d\n", ttl);

    ptr += 4;  /* skip TTL area , Get Resource length (2 bytes) */
    rs_len =  *(unsigned short*)(ptr);
    DNS_PRINTF("rs_len=%d=0x%04X\n", rs_len, rs_len);
    exchangeByteOrder((unsigned char *)&rs_len);
    DNS_PRINTF("Data length: +++++++++++++%d\n", rs_len );

    ptr += 2;   /* skip Resource length area ,Get Resource Records */
    DNS_PRINTF("Data : ");
    ptr = dns_packet_rr_resovle(packet, ptr, rs_len, ipout, iplen);

    return ptr;
}

/**
 * ���� ��Ȩ����
 * packet:���ݰ�ͷ��ָ��
 * body:  ָ����Ȩ������ʼ����
 */
static const char * dns_packet_authority_resovle(const struct DnsHeader *packet, const char *body)
{
    return 0;
}

/**
 * ���� ������Ϣ
 * packet:���ݰ�ͷ��ָ��
 * body:  ָ�򸽼���Ϣ��ʼ����
 */
static const char * dns_packet_addtional_resovle(const struct DnsHeader *packet, const char *body)
{
    return 0;
}

/**
 * ���� DNS ��Ӧ���� 
 */
static int dns_packet_body_resolve(const char *packet, char ipout[][MAX_IP_LEN])
{
    const struct DnsHeader *header;
    const char *ptr;
    unsigned short cnt;

    header = (struct DnsHeader *)packet;
    ptr = (const char *)header+sizeof(struct DnsHeader);

    DNS_PRINTF("Queston count is %d\n", header->qdCount);
    DNS_PRINTF("Answer count is %d\n", header->anCount);
    DNS_PRINTF("Authoritative count is %d\n", header->nsCount);
    DNS_PRINTF("Addtional count is %d\n", header->arCount);

    /* �������������Ϊ0������������� */
    if(header->qdCount > 0) {
        cnt = header->qdCount;
        while(cnt--){ 
            DNS_PRINTF("\n=================question_resovle %d==============\n", header->qdCount-cnt);
            ptr = dns_packet_question_resovle(header, ptr);
        }
    }

    /* ����ش�������Ϊ�㣬������ش��� */
    if(header->anCount > 0) { 
        char tmpip[MAX_IP_LEN] = {0};
        int tmpiplen = sizeof(tmpip);
        int ipindex = 0;
        
        cnt = header->anCount;
        while(cnt--) {
            DNS_PRINTF("\n=================response_resovle %d=============\n", header->anCount-cnt);
            ptr = dns_packet_response_resovle(header, ptr, tmpip, &tmpiplen);
            if (tmpiplen > 0 && ipindex < MAX_IP_CNT) {
                strcpy(ipout[ipindex++], tmpip);
            }
        }
    }

    /* �����Ȩ��������Ϊ�㣬�������Ȩ���� */
    if(header->nsCount > 0) {        
        cnt = header->nsCount;
        while(cnt--) {
            DNS_PRINTF("\n================authority_resovle %d=============\n", header->nsCount-cnt);
            ptr = dns_packet_authority_resovle(header, ptr);
        }
    }

    /* ���������Ϣ����Ϊ0�������������Ϣ */
    if(header->arCount > 0) {        
        cnt = header->arCount;
        while(cnt--) {
            DNS_PRINTF("\n================addtional_resovle %d=============\n", header->arCount-cnt);
            ptr = dns_packet_addtional_resovle(header, ptr);
        }
    }

    return 0;
}

/* ���� DNS ��Ӧ����ͷ */
static int dns_packet_hdr_resolve(char *rbuf, int rlen, char ipout[][MAX_IP_LEN])
{
    struct DnsHeader *header;

    header = (struct DnsHeader *)(rbuf);

    if(header->id != packet_id) {
        DNS_PRINTF("Packet don't match us\n");
        return -1;
    }

    if(header->qr != 1) {
        DNS_PRINTF("Not a response packet\n");
        return -1;
    }

    /* 16 bits �������ʽת��Ϊ ���ظ�ʽ*/
    exchangeByteOrder((unsigned char *)&header->anCount);
    exchangeByteOrder((unsigned char *)&header->qdCount);
    exchangeByteOrder((unsigned char *)&header->nsCount);
    exchangeByteOrder((unsigned char *)&header->arCount);

    if(header->anCount == 0) {
        DNS_PRINTF("Answer count is zero\n");
        return -1;
    }

    DNS_PRINTF("Now ,we get a valid DNS response, Resovle Now.......\n");

    dns_packet_body_resolve((char *)header, ipout);

    return 0;
}

static int set_recv_timeout(int sockfd, int timeouts)
{
    struct timeval tv;
    int ret = 0;

    (void)ret;
    tv.tv_sec = timeouts;
    tv.tv_usec = 0;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
    DNS_PRINTF("%s, timeouts=%d, sockfd=%d, ret=%d\n", __func__, timeouts, sockfd, ret);
    return ret;
}

static int get_ip_by_dns(const char *dns_server, const char *dns_addr, 
                    char ipout[MAX_IP_CNT][MAX_IP_LEN], int timeouts, int max_try_cnt)  
{  
    #if defined (PLATFORM_WINDOWS)
    SOCKET soc;
    SOCKADDR_IN addr,raddr; 
    WSADATA wsa;
    #endif

    #if defined (PLATFORM_LINUX)
    int soc;    
    struct sockaddr_in addr,raddr;
    #endif
    
    char rbuf[BUFSIZ],sbuf[BUFSIZ];  
    struct DnsHeader header;      
    int len = 0;
    int rlen = 0, addr_len,try_cnt = 0;
    int wlen = 0;
    int ret = -1;

    #if defined (PLATFORM_WINDOWS)
    /* Initial Ws2_32.dll by a process */
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        DNS_PRINTF("WSAStartup : Error code %d\n",WSAGetLastError());
        ret = -1;
        goto exit_entry;
    } 
    #endif

    if((soc = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {  
        DNS_PRINTF("Create socket fail!\n");  
        ret = -2;
        goto exit_entry;  
    }  

    set_recv_timeout(soc, timeouts);

    addr_len = sizeof(raddr);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;  
    addr.sin_addr.s_addr = inet_addr(dns_server);  
    addr.sin_port = htons(DNS_SERVER_PORT);  

    /* fill domain name field */
    dns_packet_hdr_construct(&header);
    memcpy(sbuf, (char *)&header, sizeof(header));

    len = dns_packet_body_construct(sbuf + sizeof(header), dns_addr);

    while(++try_cnt <= max_try_cnt) { 
        DNS_PRINTF("try to request DNS_server(%s) to resolve the DNS_addr(%s) (%d)\n", dns_server, dns_addr, try_cnt);
        
        wlen = sendto(soc, sbuf, len + sizeof(header) + 1, 0, (struct sockaddr *)&addr, sizeof(addr));
        DNS_PRINTF("DNS send to ret = %d\n", wlen);
        DNS_HEXDUMP("dns send: ", sbuf, len + sizeof(header) + 1);
        if(wlen < 0) {
            #if defined (PLATFORM_WINDOWS)
            DNS_PRINTF("sendto Error(%d)\n", WSAGetLastError());
            #endif
            #if defined (PLATFORM_LINUX)
            DNS_PRINTF("sendto Error(%d)=%s\n", errno, strerror(errno));
            #endif
            ret = -3;
            continue;
        }

        DNS_PRINTF("waiting to response... BUFSIZ=%d\n", BUFSIZ);
        #if defined (PLATFORM_WINDOWS)
        rlen = recvfrom(soc, rbuf, BUFSIZ, 0, (struct sockaddr *)&raddr, &addr_len);
        DNS_PRINTF("DNS recv from ret = %d\n", rlen);
        DNS_HEXDUMP("dns recv: ", rbuf, rlen);
        if(rlen < 0){
            DNS_PRINTF("recvfrom Error Error(%d)\n", WSAGetLastError());
            ret = -4;
            continue;
        }
        #endif
        #if defined (PLATFORM_LINUX)
        rlen = recvfrom(soc, rbuf, BUFSIZ, 0, (struct sockaddr *)&raddr, (socklen_t *)&addr_len);
        DNS_PRINTF("DNS recv from ret = %d\n", rlen);
        if(rlen < 0) {
            DNS_PRINTF("recvfrom Error Error(%d)=%s\n", errno, strerror(errno));
            ret = -4;
            continue;       
        }        
        #endif
        else {
            DNS_HEXDUMP("dns recv: ", rbuf, rlen);
            if(dns_packet_hdr_resolve(rbuf, rlen, ipout)  == 0) {
                DNS_PRINTF("OK!!! DNS request successfully\n");
                ret = 0;
                goto exit_entry;
            }
        }
    }

    if(try_cnt == max_try_cnt) {
        DNS_PRINTF("try %d times total, but still can't resolve the [%s]\n", MAX_TRY_CNT, dns_addr);
        ret = -5;
        goto exit_entry;
    }

exit_entry:

    #if defined (PLATFORM_WINDOWS)
    WSACleanup();   //clean up Ws2_32.dll  
    #endif

    #if defined (PLATFORM_LINUX)
    shutdown(soc,SHUT_RDWR);
    close(soc);
    #endif

    return ret;  
} 

//send dns parse request packet by udp !!!
static int get_host_ip_by_url_default_dns_server(const char *url, char *ip, int max_try_cnt)
{
    int i = 0;
    int j = 0;
    int cnt = ARRAY_SIZE(g_dns_server_list);
    const char *dns_server = NULL;
    const char *dns_addr = url; 
    char ip_out[MAX_IP_CNT][MAX_IP_LEN] = {0};
    int ret = -1;

    for (i = 0; i < cnt; i++) {
        rt_os_memset(ip_out, 0, sizeof(ip_out));
        dns_server = g_dns_server_list[i];
        ret = get_ip_by_dns(dns_server, dns_addr, ip_out, DNS_TIMEOUTS, max_try_cnt);   
        if (ret == 0) {
            for (j = 0; j < MAX_IP_CNT; j++) {
                DNS_PRINTF("ip out %d: %s\n", j, ip_out[j]);
            }
            if (rt_os_strlen(ip_out[0]) != 0) {
                DNS_PRINTF("get valid ip addr %s...\n", ip_out[0]);
                rt_os_strcpy(ip, ip_out[0]);
            }
            break;
        }
        DNS_PRINTF("dns ret=%d\n", ret);        
    }

    return ret;
}

static int get_host_ip_by_url_fixed_dns_server(const char *dns_server, const char *url, char *ip, int max_try_cnt)
{
    int i = 0;
    const char *dns_addr = url; 
    char ip_out[MAX_IP_CNT][MAX_IP_LEN] = {0};
    int ret = -1;

    DNS_PRINTF("%s() ... fixed dns-server=%s\n", __func__, dns_server);
    rt_os_memset(ip_out, 0, sizeof(ip_out));
    ret = get_ip_by_dns(dns_server, dns_addr, ip_out, DNS_TIMEOUTS, max_try_cnt);   
    if (ret == 0) {
        for (i = 0; i < MAX_IP_CNT; i++) {
            DNS_PRINTF("ip out %d: %s\n", i, ip_out[i]);
        }
        if (rt_os_strlen(ip_out[0]) != 0) {
            DNS_PRINTF("get valid ip addr %s...\n", ip_out[0]);
            rt_os_strcpy(ip, ip_out[0]);
        }
    }
    DNS_PRINTF("dns ret=%d\n", ret);        

    return ret;
}

static int get_host_ip_by_url(const char *dns_server, const char *url, char *ip)
{
    if (dns_server) {
        return get_host_ip_by_url_fixed_dns_server(dns_server, url, ip, MAX_TRY_CNT);
    } else {
        return get_host_ip_by_url_default_dns_server(url, ip, MAX_TRY_CNT);
    }
}

typedef unsigned int TCPIP_IPADDR_T;

/* Non-thread-safe */
static struct hostent *rt_gethostbyname_inner(const char *dns_server, const char *name)
{
    //TCPIP_HOST_HANDLE host_handle;
    static TCPIP_IPADDR_T ipaddr = 0;
    static char sipaddr[40] = {0}, *psipaddr[2] = {(char *) &ipaddr, 0}, *paliases[2] = {0, 0};
    static struct hostent hent = {0};
    struct hostent *h = NULL;
    int ret;

    /* use default DNS server */
    ret = get_host_ip_by_url(dns_server, name, sipaddr);
    if (ret) {
        DNS_PRINTF("dns fail\r\n");
        return NULL;
    }

    ipaddr = (TCPIP_IPADDR_T)inet_addr((const char *)sipaddr);
    DNS_PRINTF("ipaddr:%d ==> sipaddr:%s\r\n", ipaddr, sipaddr);
    hent.h_name         = (char *)name;
    hent.h_addrtype     = AF_INET;
    hent.h_length       = 4;
    hent.h_addr_list    = (char **)&psipaddr[0];
    hent.h_aliases      = (char **)&paliases[0];
    h = &hent;

    return h;
}

struct hostent *rt_gethostbyname(const char *name)
{
    return rt_gethostbyname_inner(NULL, name);   
}

struct hostent *rt_gethostbyname_with_dns_server(const char *dns_server, const char *name)
{
    return rt_gethostbyname_inner(dns_server, name);  
}

#endif
