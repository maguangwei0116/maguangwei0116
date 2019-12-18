
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
vpath %.c ./random
vpath %.c ./at
ifeq ($(LIBRARY_CFG_STANDARD_MODULE),y)
vpath %.c ./customer_at
endif

# Config your resource file list by += xxx.c
SRC-y 		+= src/file.c
SRC-y 		+= src/rt_manage_data.c
SRC-y 		+= src/http.c
SRC-y 		+= src/https.c
SRC-y 		+= src/http_client.c
SRC-y 		+= src/log.c
SRC-y 		+= src/rt_os.c
SRC-y 		+= src/rt_timer.c
ifeq ($(LIBRARY_PLATFORM_9X07), y)
SRC-y 		+= src/qmi_control_point.c
SRC-y 		+= src/qmi_dms.c
SRC-y 		+= src/qmi_nas.c
SRC-y 		+= src/qmi_uim.c
SRC-y 		+= src/qmi_wds.c
SRC-y 		+= src/rt_qmi.c
ifeq ($(LIBRARY_CFG_STANDARD_MODULE),y)
SRC-y 		+= src/customer_at.c
endif
endif
ifeq ($(LIBRARY_PLATFORM_ANDROID), y)
SRC-y 		+= src/rt_qmi_android.c
SRC-y 		+= src/rt_msg_queue.c
endif
SRC-y 		+= src/socket.c
SRC-y 		+= src/random.c
SRC-y 		+= src/at.c
SRC-y 		+= src/dns.c

# Config your include file path by += xxx-inc
INC-PATH 	+= ./file/
INC-PATH 	+= ./http/
INC-PATH 	+= ./os/
INC-PATH 	+= ./log/
#ifeq ($(LIBRARY_PLATFORM_9X07), y)
INC-PATH 	+= ./qualcomm/
#endif
INC-PATH 	+= ./socket/
INC-PATH 	+= ./random/
INC-PATH 	+= ./at/
ifeq ($(LIBRARY_CFG_STANDARD_MODULE),y)
INC-PATH 	+= ./customer_at/
endif

INC-y 		+= ./
INC-y 		+= $(INC-PATH)
INC-y 		+= $(addsuffix inc, $(wildcard $(INC-PATH)))

# Config your extern header files
EXTERN_HEADER_FILES += $(addsuffix *.h, $(wildcard $(INC-PATH)))

#all:
#	-$(Q)$(CP) $(addsuffix *.h, $(wildcard $(INC-PATH))) $(SDK_INSTALL_PATH)/include
############################################################################################
# Never modify from this line
############################################################################################
-include $(REDTEA_SUPPORT_REDTEA_PATH)/library.mk
############################################################################################
# Never modify end this line
############################################################################################