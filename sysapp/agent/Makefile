
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

# Config your own CFLAGS
USER_CFLAGS		+= -DRELEASE_TARGET=$(RELEASE_TARGET)

# Config your own libraries
QMI-LIB 		= dsi_netctrl dsutils qcmap_client qcmapipc qmi qmi_cci qmi_sap qmiservices
COMM-LIB 		= asn.1 platform dial pthread bridge util

############################################################################
### path
############################################################################
vpath %.c ./core/bootstrap

# Config your resource file list by += xxx.c
SRC-y 			+= src/bootstrap.c

# Config your include file path by += xxx-inc
INC-PATH 		= ./core/bootstrap/

INC-y 			+= ./
INC-y 			+= $(INC-PATH)
INC-y 			+= $(addsuffix inc, $(wildcard $(INC-PATH)))

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