//
// Created by admin on 2019-08-16.
//

/*
PROFILE DEFINITIONS AUTOMATIC TAGS ::= BEGIN
    profileFile ::= SEQUENCE { -- Tag '30'
            sharedProfile  SharedProfile,                                                     -- Tag '30'
            hashCode [APPLICATION 1] OCTET STRING (SIZE (32))         -- Tag '41'
    }

    SharedProfile ::= SEQUENCE {
            fileInfo FileInfo,                                                                          -- Tag '80'
            rootSk OCTET STRING,                                                               -- Tag '81'
            profileKey OCTET STRING,                                                         -- Tag '82'
            optProfiles SEQUENCE OF OperatorProfile                              -- Tag '83'
    }

    -- 文件属性信息
    FileInfo ::= SEQUENCE {
            operatorNum INTEGER,  -- 运营商个数
            version OCTET STRING
    }

    -- profile和profile info的信息
    OperatorProfile ::= SEQUENCE {
            profileInfo ProfileInfo,
            content  OCTET STRING
    }

    -- profileInfo 信息
    ProfileInfo ::= SEQUENCE {
            apn OCTET STRING,          -- 运营商APN
            totalNum INTEGER,         -- 卡片个数
            sequential BOOLEAN,        -- 是否连号，true连号，false 不连号
            preferredInfo SEQUENCE OF PreferredInfo OPTIONAL     -- 优选网络信息
    }

    -- 优选网络信息
    PreferredInfo ::= SEQUENCE {
            mcc OCTET STRING,
            plmn SEQUENCE OF Plmn
    }

    Plmn ::= SEQUENCE {
            rplmn  OCTET STRING OPTIONAL,
            fplmn  OCTET STRING OPTIONAL,
            hplmn  OCTET STRING OPTIONAL,
            ehplmn  OCTET STRING OPTIONAL,
            oplmn  OCTET STRING OPTIONAL
    }

    END


    BootstrapRequest ::= [PRIVATE 127] SEQUENCE { --Tag ‘FF7F’
        tbhRequest TBHRequest,
        hashCode OCTET STRING  -- sha256
    }

    TBHRequest ::= SEQUENCE {
        iccid OCTET STRING，  -- ICCID为卡片文件存储格式，例：98680021436587092143, 20位
        imsi OCTET STRING,      -- IMSI为卡片文件存储格式，例如 084906001111212299，18 位
        key OCTET STRING, -- 密文，32 位，Hex 格式
        opc OCTET STRING, -- 密文，32 位，Hex 格式
        rotation OCTET STRING, -- optional，遵从 SIM 格式，默认 4000204060，
        xoring OCTET STRING, -- optional, 遵从 SIM 格式，默认
        sqnFlag BOOLEAN，-- optional, true 代表打开 SQN 校验并使用默认配置，false 表示关闭 SQN 校验，默认 false
        rplmn OCTET STRING OPTIONAL,  -- 单个
        fplmn OCTET STRING OPTIONAL, -- 多个，同上
        hplmn OCTET STRING OPTIONAL, -- SIM 卡片文件存储格式，多个，eg：64F0100000 默认 FFFFFF0000
        ehplmn OCTET STRING OPTIONAL, -- 同上
        oplmn OCTET STRING OPTIONAL  -- 同上
    }


*/

#ifndef SMART_PROFILE_FILE_PARSING_H
#define SMART_PROFILE_FILE_PARSING_H

#include <stdio.h>
#include <stdbool.h>

#define ASN1_LENGTH_1BYTES                      0x81
#define ASN1_LENGTH_2BYTES                      0x82

#define SHARED_PROFILE                          0x30
#define HASH_CODE                               0x41
#define FILE_INFO                               0xA0
#define ROOT_SK                                 0x81
#define PROFILE_KEY                             0x82
#define OPT_PROFILES                            0xA3
#define PROFILE_VERSION                         0x81
#define PROFILE                                 0x81

int32_t init_profile_file(int32_t *arg);

int32_t selected_profile(int random,int enable_num);

#endif //SMART_PROFILE_PARSING_H
