
#ifndef __LPA_HTTPS_H__
#define __LPA_HTTPS_H__

int lpa_https_set_url(const char *server_url);
int lpa_https_post(const char *addr, const char *api, const char *body, char *buffer, int *size /* out */);
int lpa_https_close(void);

#endif /* __LPA_HTTPS_H__ */

