############################################################################################
# Never modify form this line
############################################################################################

all: $(TARGET)

$(TARGET): liba libso

info:
	@echo "COBJS=$(COBJS)"
	@echo "OBJS=$(OBJS)"
	@echo "ALL_TARGETS=$(ALL_TARGETS)"
	@echo "DEPS=$(DEPS)"
	@echo "CC=$(CC)" O=$(O)
	@echo "CFLAGS=$(CFLAGS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "SYSROOT=$(SYSROOT)"
	@echo "LIB_A_NAME=$(LIB_A_NAME)"
	@echo "LIB_SO_NAME=$(LIB_SO_NAME)"
	@echo "PWD=`pwd`"

clean:
	rm -rf $(O)

# Include sub comm makefiles
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/tool.mk
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/flags.mk
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/object.mk

libso: $(O)/$(LIB_SO_NAME)
liba: $(O)/$(LIB_A_NAME)

# Fucntion to generate version C file
define GEN_LIBRARY_VERSION_C
    echo "/* Auto-generated configuration version file, never modify it ! */" >$(1);\
    echo -e -n "#include <stdio.h>\n\n" >>$(1);\
    echo -e -n "int lib$(TARGET)_get_version(char *version, int size)\n" >>$(1);\
    echo -e -n "{\n" >>$(1);\
    echo -e -n "    snprintf(version, size, \"lib$(TARGET) version: %s\", RELEASE_TARGET_VERSION);\n" >>$(1);\
    echo -e -n "}\n" >>$(1)
endef

# Fucntion to generate version header file
define GEN_LIBRARY_VERSION_H
    echo "/* Auto-generated configuration version file, never modify it ! */" >$(1);\
    echo -e -n "\n#ifndef __LIB_$(TARGET)_H__\n" >>$(1);\
	echo -e -n "#define __LIB_$(TARGET)_H__\n\n" >>$(1);\
    echo -e -n "extern int lib$(TARGET)_get_version(char *version, int size);\n" >>$(1);\
    echo -e -n "\n" >>$(1);\
    echo -e -n "#endif\n" >>$(1);\
    echo -e -n "\n" >>$(1)
endef

define INSTALL_LIBRARY_VERSION_H
	cp -rf $(1) $(SDK_INSTALL_PATH)/include
endef

# Generate version C file
VERSION_FILE 	= $(O)/lib$(TARGET)_version.c
VERSION_H_FILE 	= $(O)/lib$(TARGET).h
SRC-y        	+= $(VERSION_FILE)

# Config your own CFLAGS
USER_CFLAGS  	= -DRELEASE_TARGET_VERSION=\"$(RELEASE_TARGET_VERSION)\"

$(VERSION_FILE):
	$(Q)$(call GEN_LIBRARY_VERSION_C,$@)
	$(Q)$(call GEN_LIBRARY_VERSION_H,$(VERSION_H_FILE))
	$(Q)$(call INSTALL_LIBRARY_VERSION_H,$(VERSION_H_FILE))

$(OBJS): $(VERSION_FILE)

$(O)/$(LIB_SO_NAME): $(ALL_TARGETS) $(OBJS)
	$($(quiet)do_link) $(LDFLAGS) -shared -Wl,-soname=$(LIB_SO_NAME) -Wl,--whole-archive $^ -Wl,--no-whole-archive -o"$@"
	$($(quiet)do_strip) --strip-all $(O)/$(LIB_SO_NAME)
	-$(Q)$(CP) -rf $@ $(SDK_INSTALL_PATH)/lib
	-$(Q)$(CP) -rf $@ $(SDK_INSTALL_PATH)/lib/$(LIBRARY_SHARED_TARGET_NAME)
	-$(Q)$(CD) $(SDK_INSTALL_PATH)/lib/; $(RM) -rf $(RELEASE_SHARED_TARGET); $(LN) -s $(LIB_SO_NAME) $(RELEASE_SHARED_TARGET)
	@$(ECHO) ""
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) "|"
	@$(ECHO) "|   Finished building target: $(LIBRARY_SHARED_TARGET_NAME)"
	@$(ECHO) "|"
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) ""

$(O)/$(LIB_A_NAME): $(ALL_TARGETS) $(OBJS)
	$($(quiet)do_ar) cru "$@" $^
	$($(quiet)do_ranlib) "$@"
	-$(Q)$(CP) -rf $@ $(SDK_INSTALL_PATH)/lib
	-$(Q)$(CP) -rf $@ $(SDK_INSTALL_PATH)/lib/$(LIBRARY_STATIC_TARGET_NAME)
	-$(Q)$(CD) $(SDK_INSTALL_PATH)/lib/; $(RM) -rf $(RELEASE_STATIC_TARGET); $(LN) -s $(LIB_A_NAME) $(RELEASE_STATIC_TARGET)
	@$(ECHO) ""
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) "|"
	@$(ECHO) "|   Finished building target: $(LIBRARY_STATIC_TARGET_NAME)"
	@$(ECHO) "|"
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) ""

.PHONY: all clean info $(TARGET) liba libso

# Include the dependency files, should be the last of the makefile
-include $(DEPS)

############################################################################################
# Never modify end this line
############################################################################################