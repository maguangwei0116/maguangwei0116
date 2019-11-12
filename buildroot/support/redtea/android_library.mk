
############################################################################################
# Never modify form this line
############################################################################################

all: $(TARGET)

LIB_ANDROID_AGENT_SO_NAME           = libagent.so

ANDROID_AGENT_ORG_OBJS				= $(patsubst %/main.o, , $(OBJS))  # delete main.o
ANDROID_LIBRARIES_OBJS              = $(shell find $(O)/../../library -name *.o)
ANDROID_AGENT_OBJS                  = $(ANDROID_LIBRARIES_OBJS) $(ANDROID_AGENT_ORG_OBJS)

android_info:
	@echo "ANDROID_AGENT_OBJS=$(ANDROID_AGENT_OBJS)"

# Include sub comm makefiles
-include $(REDTEA_SUPPORT_REDTEA_PATH)/tool.mk

$(TARGET): $(O)/$(LIB_ANDROID_AGENT_SO_NAME) $(O)/$(LIB_ANDROID_BOOTSTRAP_SO_NAME)

$(O)/$(LIB_ANDROID_AGENT_SO_NAME): $(ANDROID_AGENT_OBJS)
	$($(quiet)do_link) -shared -Wl,-soname=$(LIB_ANDROID_AGENT_SO_NAME) $(ANDROID_AGENT_OBJS) -o"$@"
	$($(quiet)do_strip) --strip-all $(O)/$(LIB_ANDROID_AGENT_SO_NAME)
	-$(Q)$(CP) -rf $@ $(SDK_INSTALL_PATH)/lib
	@$(ECHO) ""
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) "|"
	@$(ECHO) "|   Finished building target: $(LIB_ANDROID_AGENT_SO_NAME)"
	@$(ECHO) "|"
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) ""

.PHONY: all android_info $(TARGET)

# Include the dependency files, should be the last of the makefile
-include $(DEPS)

############################################################################################
# Never modify end this line
############################################################################################