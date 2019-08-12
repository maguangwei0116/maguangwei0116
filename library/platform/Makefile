
# Config your target here !
TARGET 		= $(shell pwd | sed 's/^\(.*\)[/]//')
LIB_SO_NAME = lib$(TARGET).so
LIB_A_NAME  = lib$(TARGET).a


############################################################################
### path
############################################################################
vpath %.c ./api/src
vpath %.c ./file/src
vpath %.c ./http/src
vpath %.c ./log/src
vpath %.c ./os/src
vpath %.c ./qualcomm/src
vpath %.c ./socket/src

# Config your resource header path and include header path

INC-PATH	= ./inc

# Config your resource file list by += xxx.c
SRC-y 		+= file.c
SRC-y 		+= rt_manage_data.c
SRC-y 		+= http.c
SRC-y 		+= log.c
SRC-y 		+= rt_os.c
SRC-y 		+= qmi_control_point.c
SRC-y 		+= qmi_dms.c
SRC-y 		+= qmi_nas.c
SRC-y 		+= qmi_uim.c
SRC-y 		+= qmi_wds.c
SRC-y 		+= rt_qmi.c

# Config your include file path by += xxx-inc
INC-y 		+= ./
INC-y 		+= ./api/
INC-y 		+= ./api/$(INC-PATH)
INC-y 		+= ./file/
INC-y 		+= ./file/$(INC-PATH)
INC-y 		+= ./http/
INC-y 		+= ./http/$(INC-PATH)
INC-y 		+= ./os/
INC-y 		+= ./os/$(INC-PATH)
INC-y 		+= ./log/
INC-y 		+= ./log/$(INC-PATH)
INC-y 		+= ./qualcomm/
INC-y 		+= ./qualcomm/$(INC-PATH)
INC-y 		+= ./socket/
INC-y 		+= ./socket/$(INC-PATH)

############################################################################################
# Never modify from this line
############################################################################################
-include ../../buildroot/support/redtea/library.mk
############################################################################################
# Never modify end this line
############################################################################################