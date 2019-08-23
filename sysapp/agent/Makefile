
############################################################################################
# Never modify from this line
############################################################################################
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/br2_conf.mk
############################################################################################
# Never modify end this line
############################################################################################

# Config your target here !
TARGET 			= $(shell pwd | sed 's/^\(.*\)[/]//')

# Config your environment, [staging] or [prod]
ENV_TYPE		= prod

# Config your sysapp version: (VERSION_MAJOR).(VERSION_MINOR).(VERSION_REVSION).(VERSION_MODIFY)
AGENT_LOCAL_VERSION	= 1.2.4.6

# Check input tag version
ifdef AGENT_VERSION
ifneq ($(AGENT_VERSION),$(AGENT_LOCAL_VERSION))
$(error "Input tag version error (tag version v$(AGENT_VERSION), local version v$(AGENT_LOCAL_VERSION))!")
endif
endif

# Config your final release target name
RELEASE_TARGET 	= $(SYSAPP_TARGET_NAME)_v$(AGENT_LOCAL_VERSION)_$(PLATFORM_TYPE)_$(ENV_TYPE)_$(DEBUG_TYPE)

# Config your application target version with git hash and build time
RELEASE_TARGET_VERSION = $(RELEASE_TARGET)_$(SYSAPP_GIT_HASH)_$(shell date +%Y-%m-%d_%H:%M:%S)

# Config your own CFLAGS
USER_CFLAGS		+= -DRELEASE_TARGET_VERSION=$(RELEASE_TARGET_VERSION)

# Config your own libraries
QMI-LIB 		= dsi_netctrl dsutils qcmap_client qcmapipc qmi qmi_cci qmi_sap qmiservices
COMM-LIB 		= comm crypto ssl pthread

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

# Config your resource file list by += xxx.c
SRC-$(CFG_AGENT_BOOTSRRAP_ON) 			+= src/bootstrap.c
SRC-$(CFG_AGENT_OTA_ON) 				+= src/ota.c
SRC-$(CFG_AGENT_PERSONALISE_ON) 		+= src/personalise.c
SRC-$(CFG_AGENT_PERSONALISE_ON) 		+= $(wildcard ./core/mqtt/src/*.c)
SRC-$(CFG_AGENT_PERSONALISE_ON) 		+= src/config.c

SRC-y 			+= src/agent_main.c
SRC-y 			+= src/agent_queue.c
SRC-y 			+= src/card_manager.c
SRC-y 			+= src/ipc_socket_client.c
SRC-y 			+= src/network_detection.c
SRC-y 			+= src/lpa.c
SRC-y 			+= src/lpdd.c
SRC-y 			+= src/luid.c
SRC-y 			+= src/ldsd.c
SRC-y 			+= src/apdu.c

# Config your include file path by += xxx-inc
INC-PATH-$(CFG_AGENT_BOOTSRRAP_ON)		+= ./core/bootstrap/
INC-PATH-$(CFG_AGENT_OTA_ON)			+= ./core/ota/
INC-PATH-$(CFG_AGENT_PERSONALISE_ON)	+= ./core/personalise/
INC-PATH-y								+= ./core/mqtt/
INC-PATH-y								+= ./core/config/
INC-PATH-y 		+= ./core/queue/
INC-PATH-y 		+= ./core/cardm/
INC-PATH-y 		+= ./core/networkd/
INC-PATH-y 		+= ./main/
INC-PATH-y 		+= ./ipc/
INC-PATH-y 		+= ./lpa/

INC-y 			+= ./
INC-y 			+= $(INC-PATH-y)
INC-y 			+= $(addsuffix inc, $(wildcard $(INC-PATH-y)))

# Config your lib
LIB-y 			= -lm
LIB-y 			+= $(patsubst %,-l%,$(COMM-LIB))
LIB-y 			+= $(patsubst %,-l%,$(QMI-LIB))

############################################################################################
# Never modify from this line
############################################################################################
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/sysapp.mk
############################################################################################
# Never modify end this line
############################################################################################