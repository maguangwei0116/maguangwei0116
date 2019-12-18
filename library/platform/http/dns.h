/**
 * file dns.h
 * Define the DNS request packet header format.
 * 2019.12.16
 */

#ifndef __DNS_H__
#define __DNS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <netdb.h>
#include <netinet/in.h>

#include "rt_type.h"

/* WARN: API non-thread-safe, the same usage as gethostbyname */
struct hostent *rt_gethostbyname(const char *name);

/* WARN: API non-thread-safe, get ip with fixed dns server */
struct hostent *rt_gethostbyname_with_dns_server(const char *dns_server, const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

