
############################################################################################
# Never modify from this line
############################################################################################
-include $(REDTEA_SUPPORT_REDTEA_PATH)/br2_conf.mk
############################################################################################
# Never modify end this line
############################################################################################

# Config your target here !
TARGET                              = $(shell pwd | sed 's/^\(.*\)[/]//')

# Config your environment, [staging] or [prod]
ENV_TYPE                            = $(SYSAPP_ENV_TYPE)

# Config your sysapp version: (VERSION_MAJOR).(VERSION_MINOR).(VERSION_REVSION).(VERSION_MODIFY)
LOCAL_TARGET_VERSION                = 0.0.0.12

# Check input tag version
ifdef MONITOR_VERSION
ifneq ($(MONITOR_VERSION),$(LOCAL_TARGET_VERSION))
$(error "Input tag version error (tag version v$(MONITOR_VERSION), local version v$(LOCAL_TARGET_VERSION))!")
endif
endif

# Config your final release target name
LOCAL_TARGET_NAME                   = $(SYSAPP_TARGET_NAME)
LOCAL_TARGET_PLATFORM_TYPE          = $(PLATFORM_TYPE)
LOCAL_TARGET_RELEASE                = $(SYSAPP_TARGET_NAME)_v$(LOCAL_TARGET_VERSION)_$(PLATFORM_TYPE)_$(ENV_TYPE)_$(DEBUG_TYPE)

# Config your application target version with git hash and build time
LOCAL_TARGET_RELEASE_VERSION_NAME   = $(LOCAL_TARGET_RELEASE)_$(SYSAPP_GIT_HASH)_$(shell date +%Y-%m-%d_%H:%M:%S)

# Config your own MACROs in CFLAGS
LOCAL_CFLAGS                        += -DLOCAL_TARGET_RELEASE_VERSION_NAME=\"$(LOCAL_TARGET_RELEASE_VERSION_NAME)\"
LOCAL_CFLAGS                        += -DLOCAL_TARGET_NAME=\"$(LOCAL_TARGET_NAME)\"
LOCAL_CFLAGS                        += -DLOCAL_TARGET_VERSION=\"$(LOCAL_TARGET_VERSION)\"
LOCAL_CFLAGS                        += -DLOCAL_TARGET_PLATFORM_TYPE=\"$(LOCAL_TARGET_PLATFORM_TYPE)\"

QMI-LIB                             = dsi_netctrl dsutils qcmapipc qmi qmi_cci qmi_sap qmiservices
ifeq ($(CFG_STANDARD_MODULE),y)
QMI-LIB                             += ql_softsim_extsdk
endif
COMM-LIB                            = crypto ssl pthread m dl
VUICC-LIB                           = xUICC skb stdc++
############################################################################
### path
############################################################################
vpath %.c ./ipc
vpath %.c ./vuicc
vpath %.c ./cover

SRC-PATH	= ./src
INC-PATH	= ./inc

# Config your resource file list by += xxx.c
SRC-y 		+= $(SRC-PATH)/vuicc_callback.c
SRC-y 		+= $(SRC-PATH)/cb_io.c
SRC-y 		+= $(SRC-PATH)/vuicc_apdu.c
SRC-y 		+= $(SRC-PATH)/ipc_socket_server.c
SRC-y 		+= $(SRC-PATH)/main.c
SRC-y 		+= ./core/backup/src/parse_backup.c
SRC-y 		+= ./core/backup/src/apdu.c
SRC-y 		+= ./core/inspect/src/inspect_file.c
SRC-y 		+= ./core/networkd/src/network_detection.c
SRC-y 		+= ./core/download/src/download_file.c

# Config your include file path by += xxx-inc
INC-y 		+= ./ipc/
INC-y 		+= ./ipc/$(INC-PATH)
INC-y 		+= ./core/backup
INC-y 		+= ./core/backup/$(INC-PATH)
INC-y 		+= ./core/inspect
INC-y 		+= ./core/inspect/$(INC-PATH)
INC-y 		+= ./core/networkd
INC-y 		+= ./core/networkd/$(INC-PATH)
INC-y 		+= ./core/download
INC-y 		+= ./core/download/$(INC-PATH)
INC-y 		+= ./vuicc/
INC-y 		+= ./vuicc/$(INC-PATH)

# Config your libraries
LIB-y 		+= $(patsubst %,-l%,$(COMM-LIB))
LIB-y 		+= $(patsubst %,-l%,$(QMI-LIB))

# Config comm library and vuicc library, keep libvuicc.a before libcomm.a
# set LIBCOMM_STATIC to 'y'(YES) or 'n'(NO)
LIBCOMM_STATIC  = n
LIB-y           += -L$(shell pwd)/vuicc/lib
LIB-y           += $(patsubst %,-l%,$(VUICC-LIB))
ifeq ($(LIBCOMM_STATIC),y)
LIB-y           += -l$(O)/../../../system/sdk/lib/libcomm.a
else
LIB-y			+= -lcomm
endif

############################################################################################
# Never modify from this line
############################################################################################
-include $(REDTEA_SUPPORT_REDTEA_PATH)/sysapp.mk
############################################################################################
# Never modify end this line
############################################################################################