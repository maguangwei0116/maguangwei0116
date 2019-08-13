
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')
LIB_SO_NAME = lib$(TARGET).so
LIB_A_NAME  = lib$(TARGET).a


############################################################################
### path
############################################################################
vpath %.c ./file
vpath %.c ./http
vpath %.c ./log
vpath %.c ./os
vpath %.c ./qualcomm
vpath %.c ./socket

# Config your resource file list by += xxx.c
SRC-y 		+= src/file.c
SRC-y 		+= src/rt_manage_data.c
SRC-y 		+= src/http.c
SRC-y 		+= src/log.c
SRC-y 		+= src/rt_os.c
SRC-y 		+= src/qmi_control_point.c
SRC-y 		+= src/qmi_dms.c
SRC-y 		+= src/qmi_nas.c
SRC-y 		+= src/qmi_uim.c
SRC-y 		+= src/qmi_wds.c
SRC-y 		+= src/rt_qmi.c

# Config your include file path by += xxx-inc
INC-T 		 = ./file/
INC-T 		+= ./http/
INC-T 		+= ./os/
INC-T 		+= ./log/
INC-T 		+= ./qualcomm/
INC-T 		+= ./socket/

INC-y 		+= ./
INC-y 		+= $(INC-T)
INC-y 		+= $(addsuffix inc, $(wildcard $(INC-T)))

all:
	$(CP) $(addsuffix *.h, $(wildcard $(INC-T))) $(SDK_INSTALL_PATH)/include
############################################################################################
# Never modify from this line
############################################################################################
-include ../../buildroot/support/redtea/library.mk
############################################################################################
# Never modify end this line
############################################################################################