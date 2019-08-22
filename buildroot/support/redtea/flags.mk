
# Config sysroot
ifneq ($(SYSROOT),)
CFLAGS 			+= --sysroot=$(SYSROOT)
LDFLAGS			+= --sysroot=$(SYSROOT)
endif

# Config include path
CFLAGS			+= $(patsubst %,-I%,$(INC-y))
CFLAGS			+= -I$(SDK_INSTALL_PATH)/include
INCTOOL			+= $(addprefix $(SYSROOT), $(SYSINC))
CFLAGS			+= $(patsubst %,-I%,$(INCTOOL))
CFLAGS			+= $(USER_CFLAGS)

# Config link flags
LDFLAGS			+= $(LIB-y)
LDFLAGS			+= -L$(SDK_INSTALL_PATH)/lib
