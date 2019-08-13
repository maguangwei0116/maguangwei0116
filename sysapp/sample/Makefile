
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')

# Config your resource header path and include header path
SRC-PATH	= ./src/
INC-PATH	= ./inc/

# Config your resource file list by += xxx.c
SRC-y 		+= $(SRC-PATH)/test.c

# Config your include file path by += xxx-inc
INC-y 		+= ./
INC-y 		+= $(INC-PATH)

# Config your libraries which should be linked
#LIB-y 		+= -lsample

############################################################################################
# Never modify form this line
############################################################################################
-include ../../buildroot/support/redtea/sysapp.mk
############################################################################################
# Never modify end this line
############################################################################################