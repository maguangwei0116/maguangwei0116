
############################################################################################
# Never modify form this line
############################################################################################

all: $(TARGET)

$(TARGET): liba libso

LIB_ALL_SO_NAME = liball.so

info:
	@echo "COBJS=$(COBJS)"
	@echo "OBJS=$(OBJS)"
	@echo "DEPS=$(DEPS)"
	@echo "CC=$(CC)" O=$(O)
	@echo "CFLAGS=$(CFLAGS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "SYSROOT=$(SYSROOT)"
	@echo "LIB_A_NAME=$(LIB_A_NAME)"
	@echo "LIB_SO_NAME=$(LIB_SO_NAME)"

clean:
	rm -rf $(O)

# Include sub comm makefiles
-include $(REDTEA_SUPPORT_REDTEA_PATH)/tool.mk
-include $(REDTEA_SUPPORT_REDTEA_PATH)/flags.mk
-include $(REDTEA_SUPPORT_REDTEA_PATH)/object.mk
-include $(REDTEA_SUPPORT_REDTEA_PATH)/gen_conf.mk

libso: $(O)/$(LIB_SO_NAME)
liba: $(O)/$(LIB_A_NAME)

define DIFF_INSTALL_LIBRARY_HEADER_FILE
    $(shell \
    if ! [ -e $(2) ]; then\
        cp -rf $(1) $(2);\
    else\
        if [ -n "$(shell test -e $(2) && diff $(1) $(2))" ]; then\
            cp -rf $(1) $(2);\
        fi;\
    fi)
endef

define INSTALL_LIBRARY_HEADER_FILE
	$(foreach file,$(1),$(call DIFF_INSTALL_LIBRARY_HEADER_FILE,$(file),$(2)/$(notdir $(file))))
endef

ifdef EXTERN_HEADER_FILES
EXTERN_HEADER_FILES += ./*.h
else
EXTERN_HEADER_FILES = ./*.h
endif

# Every object file depend on conf-file
$(OBJS): $(conf-file)

$(O)/$(LIB_SO_NAME): $(OBJS)
	$($(quiet)do_link) $(LDFLAGS) -shared -Wl,-soname=$(LIB_SO_NAME) $(OBJS) -o"$@"
	$(STRIP_ALL) "$@"
	-$(Q)$(CP) -rf $@ $(SDK_INSTALL_PATH)/lib
	-$(Q)$(call INSTALL_LIBRARY_HEADER_FILE,$(shell ls $(EXTERN_HEADER_FILES)),$(SDK_INSTALL_PATH)/include)
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
	-$(Q)$(call INSTALL_LIBRARY_HEADER_FILE,$(shell ls $(EXTERN_HEADER_FILES)),$(SDK_INSTALL_PATH)/include)
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