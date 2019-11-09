
# Config sysroot
ifneq ($(SYSROOT),)
CFLAGS 			+= --sysroot=$(SYSROOT)
LDFLAGS			+= --sysroot=$(SYSROOT)
endif

# Config include path
CFLAGS			+= $(patsubst %,-I%,$(INC-y))
CFLAGS			+= -I$(SDK_INSTALL_PATH)/include
CFLAGS			+= $(patsubst %,-I%,$(addprefix $(SYSROOT),$(SYSINC)))

# Config user CFLAGS
CFLAGS			+= $(USER_CFLAGS)

# Config common useful CFLAGS
CFLAGS			+= -g 
CFLAGS			+= -fdiagnostics-color=auto

# Config link flags
LDFLAGS			+= $(LIB-y)
LDFLAGS			+= -L$(SDK_INSTALL_PATH)/lib
LDFLAGS			+= $(USER_LDFLAGS)
