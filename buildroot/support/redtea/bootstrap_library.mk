
############################################################################################
# Never modify form this line
############################################################################################

all: $(TARGET)

LIB_ANDROID_BOOTSTRAP_SO_NAME       = libbootstrap.so

ANDROID_LIBRARIES_OBJS              = $(shell find $(O)/../../library -name *.o)
AGENT_BOOTSTRAP_OBJS                = $(patsubst %.c, $(O)/%.o, $(SRC-SINGLE-y))
ANDROID_BOOTSTRAP_OBJS              = $(ANDROID_LIBRARIES_OBJS) $(AGENT_BOOTSTRAP_OBJS)

bootstrap_info:
	@echo "ANDROID_BOOTSTRAP_OBJS=$(ANDROID_BOOTSTRAP_OBJS)"

# Include sub comm makefiles
-include $(REDTEA_SUPPORT_REDTEA_PATH)/tool.mk

$(TARGET): $(O)/$(LIB_ANDROID_BOOTSTRAP_SO_NAME)

$(O)/$(LIB_ANDROID_BOOTSTRAP_SO_NAME): $(ANDROID_BOOTSTRAP_OBJS)
	$($(quiet)do_link) -shared -Wl,-soname=$(LIB_ANDROID_BOOTSTRAP_SO_NAME) $(ANDROID_BOOTSTRAP_OBJS) -o"$@"
	$(STRIP_ALL) "$@"
	-$(Q)$(CP) -rf $@ $(SDK_INSTALL_PATH)/lib
	@$(ECHO) ""
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) "|"
	@$(ECHO) "|   Finished building target: $(LIB_ANDROID_BOOTSTRAP_SO_NAME)"
	@$(ECHO) "|"
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) ""

.PHONY: all bootstrap_info $(TARGET)

# Include the dependency files, should be the last of the makefile
-include $(DEPS)

############################################################################################
# Never modify end this line
############################################################################################