
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')

QMI-LIB = dsi_netctrl dsutils qcmap_client qcmapipc qmi qmi_cci qmi_sap qmiservices pthread
COMM-LIB = comm
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

# Config your lib
LIB-y 		= -lm
LIB-y 		+= $(patsubst %,-l%,$(COMM-LIB))
LIB-y 		+= $(patsubst %,-l%,$(QMI-LIB))

############################################################################################
# Never modify from this line
############################################################################################
-include ../../buildroot/support/redtea/sysapp.mk
############################################################################################
# Never modify end this line
############################################################################################