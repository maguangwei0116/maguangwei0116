
############################################################################################
# Never modify form this line
############################################################################################

TARGET_FILE_NAME 	= $(TARGET)
ELF_FILE_NAME 		= $(TARGET).elf
MAP_FILE_NAME 		= $(TARGET).map
DMP_FILE_NAME 		= $(TARGET).dmp
BIN_FILE_NAME 		= $(TARGET).bin

all: $(TARGET)

$(TARGET): $(O)/$(TARGET_FILE_NAME)

info:
	@echo "COBJS=$(COBJS)"
	@echo "OBJS=$(OBJS)"
	@echo "DEPS=$(DEPS)"
	@echo "CC=$(CC)" O=$(O)
	@echo "CFLAGS=$(CFLAGS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "SYSROOT=$(SYSROOT)"
	@echo "TARGET=$(TARGET)"
	@echo "REDTEA_SUPPORT_SCRIPTS_PATH=$(REDTEA_SUPPORT_SCRIPTS_PATH)"
	@echo "SYSAPP_TARGET_NAME=$(SYSAPP_TARGET_NAME)"
	@echo "RELEASE_TARGET=$(RELEASE_TARGET)"
	
clean:
	rm -rf $(O)

# Include sub comm makefiles
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/tool.mk
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/flags.mk
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/object.mk

# Add SHA256 sum to the tail of a file
define SYSAPP_ADD_SHA256SUM
	sha256sum $(1) | awk '{ print $$1 }' | xargs echo -n >> $(1)
endef

$(O)/$(TARGET_FILE_NAME): $(OBJS)
	$($(quiet)do_cc) $(MAIN_INCLUDES) -o "$@" $(OBJS) $(LDFLAGS) -Wl,-Map=$(O)/$(MAP_FILE_NAME)
	$($(quiet)do_objdump) -l -x -d "$@" > $(O)/$(DMP_FILE_NAME)
	$($(quiet)do_copy) -O binary -S "$@" $(O)/$(BIN_FILE_NAME)
	@$(CHMOD) +x "$@"
	$($(quiet)do_strip) --strip-all "$@"
	-$(Q)$(CP) -rf $(O)/$(TARGET_FILE_NAME) $(O)/$(ELF_FILE_NAME)
	-$(Q)$(call SYSAPP_ADD_SHA256SUM,$(O)/$(TARGET_FILE_NAME))
	-$(Q)$(CP) -rf $(O)/$(TARGET_FILE_NAME) $(O)/$(RELEASE_TARGET)
	-$(Q)$(CP) -rf $(O)/$(RELEASE_TARGET) $(SYSAPP_INSTALL_PATH)/
	@$(ECHO) ""
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) "|"
	@$(ECHO) "|   Finished building target: $(TARGET_FILE_NAME)"
	@$(ECHO) "|"
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) ""

.PHONY: all clean info $(TARGET)

# Include the dependency files, should be the last of the makefile
-include $(DEPS)

############################################################################################
# Never modify end this line
############################################################################################