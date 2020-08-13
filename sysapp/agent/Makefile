
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
LOCAL_TARGET_VERSION                = 0.0.1.4

# Check input tag version
ifdef AGENT_VERSION
ifneq ($(AGENT_VERSION),$(LOCAL_TARGET_VERSION))
$(error "Input tag version error (tag version v$(AGENT_VERSION), local version v$(LOCAL_TARGET_VERSION))!")
endif
endif

# Config your final release target name
LOCAL_TARGET_NAME                   = $(SYSAPP_TARGET_NAME)
LOCAL_TARGET_OEMAPP_NAME            = $(SYSAPP_TARGET_OEMAPP_NAME)
LOCAL_TARGET_PLATFORM_TYPE          = $(PLATFORM_TYPE)
LOCAL_TARGET_RELEASE                = $(SYSAPP_TARGET_NAME)_v$(LOCAL_TARGET_VERSION)_$(PLATFORM_TYPE)_$(ENV_TYPE)_$(DEBUG_TYPE)

# Config your application target version with git hash and build time
LOCAL_TARGET_RELEASE_VERSION_NAME   = $(LOCAL_TARGET_RELEASE)_$(SYSAPP_GIT_HASH)_$(shell date +%Y-%m-%d_%H:%M:%S)

# Config your own MACROs in CFLAGS
LOCAL_CFLAGS                        += -DLOCAL_TARGET_RELEASE_VERSION_NAME=\"$(LOCAL_TARGET_RELEASE_VERSION_NAME)\"
LOCAL_CFLAGS                        += -DLOCAL_TARGET_NAME=\"$(LOCAL_TARGET_NAME)\"
LOCAL_CFLAGS                        += -DLOCAL_TARGET_OEMAPP_NAME=\"$(LOCAL_TARGET_OEMAPP_NAME)\"
LOCAL_CFLAGS                        += -DLOCAL_TARGET_VERSION=\"$(LOCAL_TARGET_VERSION)\"
LOCAL_CFLAGS                        += -DLOCAL_TARGET_PLATFORM_TYPE=\"$(LOCAL_TARGET_PLATFORM_TYPE)\"

# Config MQTTS (MQTT with SSL/TLS)
LOCAL_CFLAGS                        += -DOPENSSL=1

# Config your own libraries
ifeq ($(SYSAPP_PLATFORM_9X07), y)
QMI-LIB                             = dsi_netctrl dsutils qmi_cci qmiservices
COMM-LIB                            = comm ssl crypto pthread dl
ifeq ($(CFG_STANDARD_MODULE),y)
QMI-LIB                             += ql_softsim_extsdk
endif

endif
ifeq ($(SYSAPP_PLATFORM_ANDROID), y)
COMM-LIB                            = comm z ssl crypto
PTHREAD-LIB                         = pthread
LOCAL_CFLAGS                        += -fdata-sections -ffunction-sections
ifeq ($(CFG_PLATFORM_ANDROID_ARM32), y)
LOCAL_LDFLAGS                       += $(SYSROOT)/usr/lib/arm-linux-androideabi/libc.a
LOCAL_LDFLAGS                       += -Wl,-gc-sections -Wl,-T gcc_android_arm32_ld.script
endif
ifeq ($(CFG_PLATFORM_ANDROID_ARM64), y)
LOCAL_LDFLAGS                       += -Wl,-gc-sections -Wl,-T gcc_android_arm64_ld.script
endif
endif

############################################################################
### path
############################################################################
vpath %.c ./core/bootstrap
vpath %.c ./core/ota
vpath %.c ./core/personalise
vpath %.c ./core/mqtt
vpath %.c ./core/config
vpath %.c ./core/queue
vpath %.c ./core/cardflow
vpath %.c ./core/cardm
vpath %.c ./core/logm
vpath %.c ./core/usrdata
vpath %.c ./core/networkd
vpath %.c ./main
vpath %.c ./ipc
vpath %.c ./lpa
vpath %.c ./core/ota
vpath %.c ./core/personalise

############################################################################
# Never adjustment order of these files
SRC-y                                   += ./core/upload/src/upload_null_start.c
SRC-y                                   += ./core/downstream/src/downstream_null_start.c
############################################################################

# Config your resource file list by += xxx.c
SRC-$(CFG_AGENT_BOOTSRRAP_ON)           += ./core/bootstrap/src/bootstrap.c
SRC-$(CFG_AGENT_BOOTSRRAP_ON)           += ./core/bootstrap/src/profile_parse.c

ifeq ($(CFG_AGENT_SINGLE_BOOTSRRAP_ON), y)
SRC-SINGLE-$(CFG_AGENT_BOOTSRRAP_ON)    += ./core/bootstrap/src/bootstrap.c
SRC-SINGLE-$(CFG_AGENT_BOOTSRRAP_ON)    += ./core/bootstrap/src/profile_parse.c
endif

SRC-y                                   += $(wildcard ./core/mqtt/src/*.c)
SRC-y                                   += src/config.c
SRC-y                                   += src/mbn.c
SRC-y                                   += src/agent_main.c
SRC-y                                   += src/agent_queue.c
SRC-y                                   += src/card_manager.c
SRC-y                                   += src/card_detection.c
SRC-y                                   += src/card_flow_control.c
SRC-y                                   += src/msg_process.c
SRC-y                                   += src/ipc_socket_client.c
SRC-y                                   += src/network_detection.c
SRC-y                                   += src/ping_task.c
SRC-y                                   += src/lpa.c
SRC-y                                   += src/lpdd.c
SRC-y                                   += src/luid.c
SRC-y                                   += src/ldsd.c
SRC-y                                   += src/apdu.c
SRC-y                                   += src/lpa_https.c

# Add your upload-xxx cmd here
SRC-y                                   += ./core/upload/src/upload.c
SRC-y                                   += ./core/upload/src/upload_registered.c
SRC-y                                   += ./core/upload/src/upload_boot.c
SRC-y                                   += ./core/upload/src/upload_info.c
SRC-y                                   += ./core/upload/src/upload_init.c

# Add your downstream-xxx cmd here
SRC-y                                   += ./core/downstream/src/downstream.c
SRC-y                                   += ./core/downstream/src/downstream_inspect.c
SRC-y                                   += ./core/downstream/src/downstream_reboot.c
SRC-y                                   += ./core/downstream/src/downstream_reset.c
SRC-y                                   += ./core/cardm/src/msg_push_ac_cmd.c
SRC-y                                   += ./core/cardm/src/msg_delete_cmd.c
SRC-y                                   += ./core/cardm/src/msg_enable_cmd.c
SRC-y                                   += ./core/cardm/src/msg_update_cmd.c
SRC-y                                   += ./core/networkd/src/upload_network.c

# Personalise module
SRC-$(CFG_AGENT_PERSONALISE_ON)         += ./core/personalise/src/personalise.c
SRC-$(CFG_AGENT_PERSONALISE_ON)         += ./core/personalise/src/device_info.c
SRC-$(CFG_AGENT_PERSONALISE_ON)         += ./core/personalise/src/upload_no_cert.c
SRC-$(CFG_AGENT_PERSONALISE_ON)         += ./core/personalise/src/downstream_issue_cert.c

# OTA module
SRC-$(CFG_AGENT_OTA_ON)                 += ./core/ota/src/upgrade.c
SRC-$(CFG_AGENT_OTA_ON)                 += ./core/ota/src/ota_upgrade.c

# logger manager module
SRC-y                                   += ./core/logm/src/logm.c

# usrdata manager module
SRC-y                                   += ./core/usrdata/src/usrdata.c

# agent 2 monitor
SRC-y                                   += ./core/agent2monitor/src/agent2monitor.c

# at module
SRC-y                                   += ./core/at/src/at_command.c

# agent main
SRC-y                                   += ./main/src/main.c

############################################################################
# Never adjustment order of these files
SRC-y                                   += ./core/upload/src/upload_null_end.c
SRC-y                                   += ./core/downstream/src/downstream_null_end.c
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
INC-PATH-y 		                        += ./core/ota/
INC-PATH-y 		                        += ./core/downstream/
INC-PATH-y 		                        += ./core/logm/
INC-PATH-y 		                        += ./core/agent2monitor/
INC-PATH-y 		                        += ./core/at/
INC-PATH-y 		                        += ./core/cardflow/
INC-PATH-y 		                        += ./core/usrdata/

INC-y 			                        += ./
INC-y 			                        += $(INC-PATH-y)
INC-y 			                        += $(addsuffix inc, $(wildcard $(INC-PATH-y)))

# Config your lib
LIB-y 			                        += -lm
LIB-y 			                        += $(patsubst %,-l%,$(COMM-LIB))
LIB-y 			                        += $(patsubst %,-l%,$(QMI-LIB))
ifeq ($(SYSAPP_PLATFORM_ANDROID), y)
LIB-y 			                        += $(patsubst %,-%,$(PTHREAD-LIB))
endif

############################################################################################
# Never modify from this line
############################################################################################
-include $(REDTEA_SUPPORT_REDTEA_PATH)/sysapp.mk
ifeq ($(CFG_AGENT_SINGLE_BOOTSRRAP_ON), y)
-include $(REDTEA_SUPPORT_REDTEA_PATH)/bootstrap_library.mk
endif
ifeq ($(SYSAPP_PLATFORM_ANDROID), y)
-include $(REDTEA_SUPPORT_REDTEA_PATH)/android_library.mk
endif
ifeq ($(CFG_TEST_LPA_ON), y)
-include $(REDTEA_SUPPORT_REDTEA_PATH)/test_lpa_sysapp.mk
endif
############################################################################################
# Never modify end this line
############################################################################################