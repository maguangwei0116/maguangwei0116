
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')

QMI-LIB = dsi_netctrl dsutils qcmap_client qcmapipc qmi qmi_cci qmi_sap qmiservices
COMM-LIB = comm crypto ssl pthread

############################################################################
### path
############################################################################
vpath %.c ./core/bootstrap
vpath %.c ./core/queue
vpath %.c ./core/cardm
vpath %.c ./core/logm
vpath %.c ./main
vpath %.c ./ipc
vpath %.c ./lpa

# Config your resource file list by += xxx.c
SRC-y 			+= src/bootstrap.c
SRC-y 			+= src/agent_main.c
SRC-y 			+= src/agent_queue.c
SRC-y 			+= src/card_manager.c
SRC-y 			+= src/ipc_socket_client.c
SRC-y 			+= src/lpa.c
SRC-y 			+= src/lpdd.c
SRC-y 			+= src/luid.c
SRC-y 			+= src/ldsd.c
SRC-y 			+= src/apdu.c

# Config your include file path by += xxx-inc
INC-PATH 		= ./core/bootstrap/
INC-PATH 		+= ./core/queue/
INC-PATH 		+= ./core/cardm/
INC-PATH 		+= ./main/
INC-PATH 		+= ./ipc/
INC-PATH 		+= ./lpa/

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
-include ../../buildroot/support/redtea/sysapp.mk
############################################################################################
# Never modify end this line
############################################################################################