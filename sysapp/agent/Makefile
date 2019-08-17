
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')

QMI-LIB = dsi_netctrl dsutils qcmap_client qcmapipc qmi qmi_cci qmi_sap qmiservices
COMM-LIB = asn.1 platform dial pthread bridge util
############################################################################
### path
############################################################################
vpath %.c ./core/bootstrap

# Config your resource file list by += xxx.c
SRC-y 		+= src/bootstrap.c

# Config your include file path by += xxx-inc
INC-PATH 		 = ./core/bootstrap/

INC-y 		+= ./
INC-y 		+= $(INC-PATH)
INC-y 		+= $(addsuffix inc, $(wildcard $(INC-PATH)))

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