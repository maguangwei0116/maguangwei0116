
# Config sysroot
ifneq ($(SYSROOT),)
CFLAGS 			+= --sysroot=$(SYSROOT)
LDFLAGS			+= --sysroot=$(SYSROOT)
endif

# Config include path
INCDIR  		= $(patsubst %,-I%,$(INC-y))
CFLAGS			+= $(INCDIR)
CFLAGS			+= -I$(SDK_INSTALL_PATH)/include
INCTOOL			+= $(addprefix $(SYSROOT), $(SYSINC))
CFLAGS			+= $(patsubst %,-I%,$(INCTOOL))

# Config link flags
LDFLAGS			+= $(LIB-y)
LDFLAGS			+= -L$(SDK_INSTALL_PATH)/lib
