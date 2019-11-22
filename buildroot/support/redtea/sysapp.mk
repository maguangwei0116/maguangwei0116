
############################################################################################
# Never modify form this line
############################################################################################

TARGET_FILE_NAME 	= $(TARGET)
ELF_FILE_NAME 		= $(TARGET).elf
MAP_FILE_NAME 		= $(TARGET).map
DMP_FILE_NAME 		= $(TARGET).dmp
BIN_FILE_NAME 		= $(TARGET).bin

all: $(TARGET)

$(TARGET): $(O)/$(TARGET_FILE_NAME) generate_signature

info:
	@echo "COBJS=$(COBJS)"
	@echo "OBJS=$(OBJS)"
	@echo "DEPS=$(DEPS)"
	@echo "CC=$(CC)" O=$(O)
	@echo "CFLAGS=$(CFLAGS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "SYSROOT=$(SYSROOT)"
	@echo "TARGET=$(TARGET)"
	@echo "REDTEA_SUPPORT_REDTEA_PATH=$(REDTEA_SUPPORT_REDTEA_PATH)"
	@echo "SYSAPP_TARGET_NAME=$(SYSAPP_TARGET_NAME)"
	@echo "LOCAL_TARGET_RELEASE=$(LOCAL_TARGET_RELEASE)"
#	@echo "$(.VARIABLES)"
	@echo "BR2_CONF_MK=$(BR2_CONF_MK)"

clean:
	rm -rf $(O)

# Include sub comm makefiles
-include $(REDTEA_SUPPORT_REDTEA_PATH)/tool.mk
-include $(REDTEA_SUPPORT_REDTEA_PATH)/flags.mk
-include $(REDTEA_SUPPORT_REDTEA_PATH)/object.mk
-include $(REDTEA_SUPPORT_REDTEA_PATH)/gen_conf.mk

# Add SHA256withECC signature to the tail of a file
define SYSAPP_ADD_SHA256withECC
	$(REDTEA_SUPPORT_SCRIPTS_PATH)/sign_file.sh $(1) $(REDTEA_SUPPORT_SCRIPTS_PATH) Q=$(Q)
endef

generate_signature: $(O)/$(TARGET_FILE_NAME)
	$(Q)$(call SYSAPP_ADD_SHA256withECC,$(O)/$(TARGET_FILE_NAME))
	-$(Q)$(CP) -rf $(O)/$(TARGET_FILE_NAME) $(O)/$(LOCAL_TARGET_RELEASE)
	-$(Q)$(CP) -rf $(O)/$(LOCAL_TARGET_RELEASE) $(SYSAPP_INSTALL_PATH)/
	@$(ECHO) ""
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) "|"
	@$(ECHO) "|   Finished building target: $(LOCAL_TARGET_RELEASE)"
	@$(ECHO) "|"
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) ""

# Every object file depend on conf-file
$(OBJS): $(conf-file)

$(O)/$(TARGET_FILE_NAME): $(OBJS)
	$($(quiet)do_link) -o "$@" -Wl,--whole-archive $(OBJS) -Wl,--no-whole-archive $(LDFLAGS) -Wl,-Map=$(O)/$(MAP_FILE_NAME) 
	$($(quiet)do_objdump) -l -x -d "$@" > $(O)/$(DMP_FILE_NAME)
	$($(quiet)do_copy) -O binary -S "$@" $(O)/$(BIN_FILE_NAME)
	@$(CHMOD) +x "$@"
	$($(quiet)do_strip) --strip-all "$@"
	-$(Q)$(CP) -rf $(O)/$(TARGET_FILE_NAME) $(O)/$(ELF_FILE_NAME)

.PHONY: all clean info $(TARGET) generate_signature FORCE

# Include the dependency files, should be the last of the makefile
-include $(DEPS)

############################################################################################
# Never modify end this line
############################################################################################