//
// Created by admin on 2019-09-06.
//

#ifndef SMART_DOWNSTREAM_ISSUE_CERT_H
#define SMART_DOWNSTREAM_ISSUE_CERT_H

typedef struct issue_cert_struct {
#define MAX_TRANID_LEN              128
#define MAX_FILE_HASH_LEN           64
#define MAX_TICKET_LEN              32
#define HASH_CHECK_BLOCK            1024  // 哈希校验每块的大小

    int8_t tranid[MAX_TRANID_LEN + 1];
    int8_t fileHash[MAX_FILE_HASH_LEN + 1];  // 平台下载文件的hash码
    int8_t ticket[MAX_TICKET_LEN + 1];
} issue_cert_struct_t;

#endif //SMART_DOWNSTREAM_ISSUE_CERT_H
