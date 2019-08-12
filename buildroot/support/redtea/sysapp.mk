
############################################################################################
# Never modify form this line
############################################################################################

TARGET_FILE_NAME 	= $(TARGET)
ELF_FILE_NAME 		= $(TARGET).elf
MAP_FILE_NAME 		= $(TARGET).map
DMP_FILE_NAME 		= $(TARGET).dmp
BIN_FILE_NAME 		= $(TARGET).bin

all: $(TARGET)

$(TARGET): info $(O)/$(TARGET_FILE_NAME)

info:
	@echo "COBJS=$(COBJS)"
	@echo "OBJS=$(OBJS)"
	@echo "DEPS=$(DEPS)"
	@echo "CC=$(CC)" O=$(O) -DMACRO=$(MACRO)
	@echo "CFLAGS=$(CFLAGS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "SYSROOT=$(SYSROOT)"
	@echo "TARGET=$(TARGET)"
clean:
	rm -rf $(O)

# Include sub comm makefiles
-include ../../buildroot/support/redtea/tool.mk
-include ../../buildroot/support/redtea/flags.mk
-include ../../buildroot/support/redtea/object.mk

$(O)/$(TARGET_FILE_NAME): $(OBJS)
	$($(quiet)do_cc) $(MAIN_INCLUDES) -o "$@" $(OBJS) $(LDFLAGS) -Wl,-Map=$(O)/$(MAP_FILE_NAME)
	$($(quiet)do_objdump) -l -x -d "$@" > $(O)/$(DMP_FILE_NAME)
	$($(quiet)do_copy) -O binary -S "$@" $(O)/$(BIN_FILE_NAME)
	@$(CHMOD) +x "$@"
	$($(quiet)do_strip) --strip-all "$@"
	-$(Q)$(CP) -rf $(O)/$(TARGET_FILE_NAME) $(O)/$(ELF_FILE_NAME)
	-$(Q)$(CP) -rf $@ $(SYSAPP_INSTALL_PATH)/
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