
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')

# Config your environment, [staging] or [prod]
ENV_TYPE		= prod

# Config your sysapp version: (VERSION_MAJOR).(VERSION_MINOR).(VERSION_REVSION).(VERSION_MODIFY)
AGENT_LOCAL_VERSION	= 1.0.0.0

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

QMI-LIB = dsi_netctrl dsutils qcmap_client qcmapipc qmi qmi_cci qmi_sap qmiservices
COMM-LIB = comm crypto ssl pthread
VUICC-LIB = libvuicc.a
############################################################################
### path
############################################################################
vpath %.c ./ipc
vpath %.c ./vuicc
vpath %.c ./cover

SRC-PATH	= ./src
INC-PATH	= ./inc

# Config your resource file list by += xxx.c
SRC-y 		+= $(SRC-PATH)/ipc_socket_server.c
SRC-y 		+= $(SRC-PATH)/main.c

# Config your include file path by += xxx-inc
INC-y 		+= ./ipc/
INC-y 		+= ./ipc/$(INC-PATH)
INC-y 		+= ./vuicc/
INC-y 		+= ./vuicc/$(INC-PATH)

# Config your lib
LIB-y 		= -lm
LIB-y 		+= -l$(shell pwd)/vuicc/lib/$(VUICC-LIB)
LIB-y 		+= $(patsubst %,-l%,$(COMM-LIB))
LIB-y 		+= $(patsubst %,-l%,$(QMI-LIB))

############################################################################################
# Never modify from this line
############################################################################################
-include ../../buildroot/support/redtea/sysapp.mk
############################################################################################
# Never modify end this line
############################################################################################