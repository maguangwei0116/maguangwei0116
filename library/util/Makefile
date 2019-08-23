
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')
LIB_SO_NAME = lib$(TARGET).so
LIB_A_NAME  = lib$(TARGET).a

# Config your resource header path and include header path
SRC-PATH	= ./src/
INC-PATH	= ./inc/

# Config your resource file list by += xxx.c
SRC-y 		+= $(SRC-PATH)/cJSON.c
SRC-y 		+= $(SRC-PATH)/convert.c
SRC-y 		+= $(SRC-PATH)/hash.c
SRC-y 		+= $(SRC-PATH)/md5.c
SRC-y 		+= $(SRC-PATH)/base64.c

# Config your include file path by += xxx-inc
INC-y 		+= ./
INC-y 		+= $(INC-PATH)

############################################################################################
# Never modify form this line
############################################################################################
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/library.mk
############################################################################################
# Never modify end this line
############################################################################################