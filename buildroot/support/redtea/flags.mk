
# Config sysroot
ifneq ($(SYSROOT),)
CFLAGS 			+= --sysroot=$(SYSROOT)
LDFLAGS			+= --sysroot=$(SYSROOT)
endif

# Config include path
CFLAGS			+= $(patsubst %,-I%,$(INC-y))
CFLAGS			+= -I$(SDK_INSTALL_PATH)/include
CFLAGS			+= $(patsubst %,-I%,$(addprefix $(SYSROOT),$(SYSINC)))
CFLAGS			+= $(USER_CFLAGS)

# Config link flags
LDFLAGS			+= $(LIB-y)
LDFLAGS			+= -L$(SDK_INSTALL_PATH)/lib
