
############################################################################################
# Never modify from this line
############################################################################################
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/br2_conf.mk
############################################################################################
# Never modify end this line
############################################################################################

# Config your target here !
TARGET                              = $(shell pwd | sed 's/^\(.*\)[/]//')

# Config your environment, [staging] or [prod]
ENV_TYPE                            = $(SYSAPP_ENV_TYPE)

# Config your sysapp version: (VERSION_MAJOR).(VERSION_MINOR).(VERSION_REVSION).(VERSION_MODIFY)
LOCAL_TARGET_VERSION                = 0.0.0.4

# Check input tag version
ifdef AGENT_VERSION
ifneq ($(AGENT_VERSION),$(LOCAL_TARGET_VERSION))
$(error "Input tag version error (tag version v$(AGENT_VERSION), local version v$(LOCAL_TARGET_VERSION))!")
endif
endif

# Config your final release target name
LOCAL_TARGET_NAME                   = $(SYSAPP_TARGET_NAME)
LOCAL_TARGET_PLATFORM_TYPE          = $(PLATFORM_TYPE)
LOCAL_TARGET_RELEASE                = $(SYSAPP_TARGET_NAME)_v$(LOCAL_TARGET_VERSION)_$(PLATFORM_TYPE)_$(ENV_TYPE)_$(DEBUG_TYPE)

# Config your application target version with git hash and build time
LOCAL_TARGET_RELEASE_VERSION_NAME   = $(LOCAL_TARGET_RELEASE)_$(SYSAPP_GIT_HASH)_$(shell date +%Y-%m-%d_%H:%M:%S)

# Config your own MACROs in CFLAGS
USER_CFLAGS                         += -DLOCAL_TARGET_RELEASE_VERSION_NAME=\"$(LOCAL_TARGET_RELEASE_VERSION_NAME)\"
USER_CFLAGS                         += -DLOCAL_TARGET_NAME=\"$(LOCAL_TARGET_NAME)\"
USER_CFLAGS                         += -DLOCAL_TARGET_VERSION=\"$(LOCAL_TARGET_VERSION)\"
USER_CFLAGS                         += -DLOCAL_TARGET_PLATFORM_TYPE=\"$(LOCAL_TARGET_PLATFORM_TYPE)\"

# Config backtrace module switch [y/n]
ifeq ($(CFG_ENABLE_LIBUNWIND),y)
USER_CFLAGS                         += -g -rdynamic -mapcs-frame -funwind-tables -fasynchronous-unwind-tables
USER_LDFLAGS                        += -lunwind -ldl
endif

# Config your own libraries
QMI-LIB                             = dsi_netctrl dsutils qcmap_client qcmapipc qmi qmi_cci qmi_sap qmiservices
COMM-LIB                            = comm crypto ssl pthread

############################################################################
### path
############################################################################
vpath %.c ./core/bootstrap
vpath %.c ./core/ota
vpath %.c ./core/personalise
vpath %.c ./core/mqtt
vpath %.c ./core/config
vpath %.c ./core/queue
vpath %.c ./core/cardm
vpath %.c ./core/logm
vpath %.c ./core/networkd
vpath %.c ./main
vpath %.c ./ipc
vpath %.c ./lpa
vpath %.c ./core/ota
vpath %.c ./core/personalise

############################################################################
# Never adjustment order of these files
SRC-y 			                        += ./core/upload/src/upload_null_start.c
SRC-y 			                        += ./core/downstream/src/downstream_null_start.c
############################################################################

# Config your resource file list by += xxx.c
SRC-$(CFG_AGENT_BOOTSRRAP_ON) 			+= src/bootstrap.c
SRC-$(CFG_AGENT_BOOTSRRAP_ON) 			+= src/profile_parse.c
SRC-y 		                            += $(wildcard ./core/mqtt/src/*.c)
SRC-y 		                            += src/config.c
SRC-y 			                        += src/agent_main.c
SRC-y 			                        += src/agent_queue.c
SRC-y 			                        += src/card_manager.c
SRC-y 			                        += src/msg_process.c
SRC-y 			                        += src/ipc_socket_client.c
SRC-y 			                        += src/network_detection.c
SRC-y 			                        += src/lpa.c
SRC-y 			                        += src/lpdd.c
SRC-y 			                        += src/luid.c
SRC-y 			                        += src/ldsd.c
SRC-y 			                        += src/apdu.c
SRC-y 			                        += ./core/random/src/random.c

# Add your upload-xxx cmd here
SRC-y 			                        += ./core/upload/src/upload.c
SRC-y 			                        += ./core/upload/src/upload_registered.c
SRC-y 			                        += ./core/upload/src/upload_boot.c
SRC-y 			                        += ./core/upload/src/upload_info.c
SRC-y 			                        += ./core/upload/src/upload_init.c

# Add your downstream-xxx cmd here
SRC-y 			                        += ./core/downstream/src/downstream.c
SRC-y 			                        += ./core/downstream/src/downstream_inspect.c
SRC-y 			                        += ./core/downstream/src/downstream_reboot.c
SRC-y 			                        += ./core/downstream/src/downstream_reset.c
SRC-y 			                        += ./core/cardm/src/msg_push_ac_cmd.c
SRC-y 			                        += ./core/cardm/src/msg_delete_cmd.c
SRC-y 			                        += ./core/cardm/src/msg_enable_cmd.c
SRC-y 			                        += ./core/networkd/src/upload_network.c

# Personalise module
SRC-$(CFG_AGENT_PERSONALISE_ON) 		+= src/personalise.c
SRC-$(CFG_AGENT_PERSONALISE_ON) 		+= src/device_info.c
SRC-$(CFG_AGENT_PERSONALISE_ON) 		+= src/upload_no_cert.c
SRC-$(CFG_AGENT_PERSONALISE_ON) 		+= src/downstream_issue_cert.c

# OTA module
SRC-$(CFG_AGENT_OTA_ON) 				+= src/upgrade.c
SRC-$(CFG_AGENT_OTA_ON) 				+= src/ota_upgrade.c

# logger manager module
SRC-y 			                        += ./core/logm/src/logm.c

############################################################################
# Never adjustment order of these files
SRC-y 			                        += ./core/upload/src/upload_null_end.c
SRC-y 			                        += ./core/downstream/src/downstream_null_end.c
############################################################################

# Config your include file path by += xxx-inc
INC-PATH-$(CFG_AGENT_BOOTSRRAP_ON)		+= ./core/bootstrap/
INC-PATH-$(CFG_AGENT_OTA_ON)			+= ./core/ota/
INC-PATH-$(CFG_AGENT_PERSONALISE_ON)	+= ./core/personalise/
INC-PATH-y								+= ./core/mqtt/
INC-PATH-y								+= ./core/config/
INC-PATH-y 		                        += ./core/queue/
INC-PATH-y 		                        += ./core/cardm/
INC-PATH-y 		                        += ./core/networkd/
INC-PATH-y 		                        += ./main/
INC-PATH-y 		                        += ./ipc/
INC-PATH-y 		                        += ./lpa/
INC-PATH-y 		                        += ./core/upload/
INC-PATH-y 		                        += ./core/random/
INC-PATH-y 		                        += ./core/ota/
INC-PATH-y 		                        += ./core/downstream/
INC-PATH-y 		                        += ./core/logm/

INC-y 			                        += ./
INC-y 			                        += $(INC-PATH-y)
INC-y 			                        += $(addsuffix inc, $(wildcard $(INC-PATH-y)))

# Config your lib
LIB-y 			                        += -lm
LIB-y 			                        += $(patsubst %,-l%,$(COMM-LIB))
LIB-y 			                        += $(patsubst %,-l%,$(QMI-LIB))

############################################################################################
# Never modify from this line
############################################################################################
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/sysapp.mk
############################################################################################
# Never modify end this line
############################################################################################