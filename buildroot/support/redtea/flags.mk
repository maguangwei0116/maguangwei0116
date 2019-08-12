
# Config sysroot
ifneq ($(SYSROOT),)
CFLAGS 			+= --sysroot=$(SYSROOT)
LDFLAGS			+= --sysroot=$(SYSROOT)
endif

# Config include path
INCDIR  		= $(patsubst %,-I%,$(INC-y))
CFLAGS			+= $(INCDIR)
CFLAGS			+= -I$(SDK_INSTALL_PATH)/include
CFLAGS			+= $(patsubst %,-I%,$(SYSINC))

# Config link flags
LDFLAGS			+= $(LIB-y)
LDFLAGS			+= -L$(SDK_INSTALL_PATH)/lib
