############################################################################################
# Never modify form this line
############################################################################################

all: $(TARGET)

$(TARGET): liba libso COPY_VERSION_FILE_OUT

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
    echo -e -n "    snprintf(version, size, \"lib$(TARGET) version: %s\", LOCAL_TARGET_RELEASE_VERSION_NAME);\n" >>$(1);\
    echo -e -n "    return 0;\n" >>$(1);\
    echo -e -n "}\n" >>$(1);\
    echo -e -n "int lib$(TARGET)_get_all_version(char *name, int n_size, char *version, int v_size, char *chip_modle, int c_size)\n" >>$(1);\
    echo -e -n "{\n" >>$(1);\
    echo -e -n "    snprintf(name, n_size, \"%s\", LOCAL_TARGET_NAME);\n" >>$(1);\
    echo -e -n "    snprintf(version, v_size, \"%s\", LOCAL_TARGET_VERSION);\n" >>$(1);\
    echo -e -n "    snprintf(chip_modle, c_size, \"%s\", LOCAL_TARGET_PLATFORM_TYPE);\n" >>$(1);\
    echo -e -n "    return 0;\n" >>$(1);\
    echo -e -n "}\n" >>$(1)
endef

# Fucntion to generate version header file
define GEN_LIBRARY_VERSION_H
    echo "/* Auto-generated configuration version file, never modify it ! */" >$(1);\
    echo -e -n "\n#ifndef __LIB_$(TARGET)_H__\n" >>$(1);\
    echo -e -n "#define __LIB_$(TARGET)_H__\n\n" >>$(1);\
    echo -e -n "extern int lib$(TARGET)_get_version(char *version, int size);\n" >>$(1);\
    echo -e -n "extern int lib$(TARGET)_get_all_version(char *name, int n_size, char *version, int v_size, char *chip_modle, int c_size);\n" >>$(1);\
    echo -e -n "\n" >>$(1);\
    echo -e -n "#endif\n" >>$(1);\
    echo -e -n "\n" >>$(1)
endef

define INSTALL_LIBRARY_VERSION_H
	cp -rf $(1) $(SDK_INSTALL_PATH)/include
endef

define INSTALL_LIBRARY_VERSION_C
	mv $(1) $(2)
endef

# All library objects
ALL_OBJ_TARGETS = $(shell find $(O) -name *.o)

# Generate version C/H file
VER_C_FILE      = lib$(TARGET)_version.c
VER_H_FILE      = lib$(TARGET).h
VER_C_FILE_OUT 	= $(O)/$(VER_C_FILE)
VER_H_FILE_OUT 	= $(O)/$(VER_H_FILE)
SRC-y        	+= $(VER_C_FILE)

$(VER_C_FILE): GEN_VERSION_FILE

COPY_VERSION_FILE_OUT: liba libso
	$(Q)$(call INSTALL_LIBRARY_VERSION_C,$(VER_C_FILE),$(VER_C_FILE_OUT))

GEN_VERSION_FILE:
	$(Q)$(call GEN_LIBRARY_VERSION_C,$(VER_C_FILE))
	$(Q)$(call GEN_LIBRARY_VERSION_H,$(VER_H_FILE_OUT))
	$(Q)$(call INSTALL_LIBRARY_VERSION_H,$(VER_H_FILE_OUT))

$(O)/$(LIB_SO_NAME): $(OBJS) $(ALL_TARGETS)
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

$(O)/$(LIB_A_NAME): $(OBJS) $(ALL_TARGETS) 
#	echo ALL_OBJ_TARGETS=$(ALL_OBJ_TARGETS)
	$($(quiet)do_ar) cru "$@" $(ALL_OBJ_TARGETS)
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

.PHONY: all clean info $(TARGET) liba libso $(GEN_VERSION_FILE) $(COPY_VERSION_FILE_OUT)

# Include the dependency files, should be the last of the makefile
-include $(DEPS)

############################################################################################
# Never modify end this line
############################################################################################