
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')
LIB_SO_NAME = lib$(TARGET).so
LIB_A_NAME  = lib$(TARGET).a

# Config your resource header path and include header path
SRC-PATH	= ./src
INC-PATH	= ./inc

# Config your resource file list by += xxx.c
ifeq ($(LIBRARY_PLATFORM_9X07), y)
SRC-y 		+= $(SRC-PATH)/ipc_socket_client.c
#SRC-y 		+= $(SRC-PATH)/ipc_socket_server.c
SRC-y 		+= $(SRC-PATH)/ipc_agent_client.c
endif

# Config your include file path by += xxx-inc
INC-y 		+= ./
INC-y 		+= $(INC-PATH)/

############################################################################################
# Never modify form this line
############################################################################################
-include $(REDTEA_SUPPORT_REDTEA_PATH)/library.mk
############################################################################################
# Never modify end this line
############################################################################################