
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')
LIB_SO_NAME = lib$(TARGET).so
LIB_A_NAME  = lib$(TARGET).a

# Config your resource header path and include header path
SRC-PATH	= ./src
INC-PATH	= ./inc

# Config your resource file list by += xxx.c
SRC-y 		+= $(SRC-PATH)/dial_up.c

# Config your include file path by += xxx-inc
INC-y 		+= ./
INC-y 		+= $(INC-PATH)/

############################################################################################
# Never modify form this line
############################################################################################
-include ../../buildroot/support/redtea/library.mk
############################################################################################
# Never modify end this line
############################################################################################