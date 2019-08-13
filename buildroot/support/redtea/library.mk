
############################################################################################
# Never modify form this line
############################################################################################

all: $(TARGET)

$(TARGET): liba libso

info:
	@echo "COBJS=$(COBJS)"
	@echo "OBJS=$(OBJS)"
	@echo "DEPS=$(DEPS)"
	@echo "CC=$(CC)" O=$(O) -DMACRO=$(MACRO)
	@echo "CFLAGS=$(CFLAGS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "SYSROOT=$(SYSROOT)"
	@echo "LIB_A_NAME=$(LIB_A_NAME)"
	@echo "LIB_SO_NAME=$(LIB_SO_NAME)"

clean:
	rm -rf $(O)

# Include sub comm makefiles
-include ../../buildroot/support/redtea/tool.mk
-include ../../buildroot/support/redtea/flags.mk
-include ../../buildroot/support/redtea/object.mk

libso: $(O)/$(LIB_SO_NAME)
liba: $(O)/$(LIB_A_NAME)

$(O)/$(LIB_SO_NAME): $(OBJS)
	$($(quiet)do_link) $(LDFLAGS) -shared -Wl,-soname=$(LIB_SO_NAME) $(OBJS) -o"$@"
	$($(quiet)do_strip) --strip-all $(O)/$(LIB_SO_NAME)
	-$(Q)$(CP) -rf $@ $(SDK_INSTALL_PATH)/lib
	-$(Q)$(CP) -rf ./*.h $(SDK_INSTALL_PATH)/include
	@$(ECHO) ""
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) "|"
	@$(ECHO) "|   Finished building target: $(LIB_SO_NAME)"
	@$(ECHO) "|"
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) ""

$(O)/$(LIB_A_NAME): $(OBJS)
	$($(quiet)do_ar) cru "$@" $^
	$($(quiet)do_ranlib) "$@"
	-$(Q)$(CP) -rf $@ $(SDK_INSTALL_PATH)/lib
	-$(Q)$(CP) -rf ./*.h $(SDK_INSTALL_PATH)/include
	@$(ECHO) ""
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) "|"
	@$(ECHO) "|   Finished building target: $(LIB_A_NAME)"
	@$(ECHO) "|"
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) ""

.PHONY: all clean info $(TARGET) liba libso

# Include the dependency files, should be the last of the makefile
-include $(DEPS)

############################################################################################
# Never modify end this line
############################################################################################