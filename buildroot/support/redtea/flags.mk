
# Config sysroot
ifneq ($(SYSROOT),)
CFLAGS          += --sysroot=$(SYSROOT)
LDFLAGS         += --sysroot=$(SYSROOT)
endif

# Config backtrace module switch [y/n]
ifeq ($(CFG_ENABLE_LIBUNWIND),y)
LOCAL_CFLAGS    += -g -rdynamic -mapcs-frame -funwind-tables -fasynchronous-unwind-tables
LOCAL_LDFLAGS   += -lunwind -ldl
endif

# Config include path
CFLAGS          += $(patsubst %,-I%,$(INC-y))
CFLAGS          += -I$(SDK_INSTALL_PATH)/include
CFLAGS          += $(patsubst %,-I%,$(addprefix $(SYSROOT),$(SYSINC)))

# Config user CFLAGS
CFLAGS          += $(LOCAL_CFLAGS)

# Config common useful CFLAGS
CFLAGS          += -fdiagnostics-color=auto

# Config link flags
LDFLAGS         += $(LIB-y)
LDFLAGS         += -L$(SDK_INSTALL_PATH)/lib
LDFLAGS         += $(LOCAL_LDFLAGS)

# Config compile optimize flags
#CFLAGS         += -g
#CFLAGS         += -Os
#CFLAGS         += -fdata-sections
#CFLAGS         += -ffunction-sections
#CFLAGS         += -fvisibility=hidden
#LDFLAGS        += -Wl,--gc-sections
#LDFLAGS        += -Wl,--print-gc-sections
#LDFLAGS        += -Wl,--print-map
