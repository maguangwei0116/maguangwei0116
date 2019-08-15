
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')
############################################################################
### path
############################################################################
vpath %.c ./core/bootstrap

# Config your resource file list by += xxx.c
SRC-y 		+= src/bootstrap.c

# Config your include file path by += xxx-inc
INC-T 		 = ./core/bootstrap/

INC-y 		+= ./
INC-y 		+= $(INC-T)
INC-y 		+= $(addsuffix inc, $(wildcard $(INC-T)))

LIB-y 		+= -lasn.1

############################################################################################
# Never modify from this line
############################################################################################
-include ../../buildroot/support/redtea/sysapp.mk
############################################################################################
# Never modify end this line
############################################################################################