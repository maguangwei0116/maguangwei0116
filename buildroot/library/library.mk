
TARGETS += library

ifeq ($(BR2_PACKAGE_LIBRARY),y)

LIBRARY_DIR=$(BUILD_DIR)/library
LIBRARY_SOURCE_PATH=$(call qstrip,$(BR2_LIBRARY_SOURCE_PATH))
LIBRARY_INSTALL_PATH=$(call qstrip,$(BR2_LIBRARY_INSTALL_PATH))
LIBRARY_SUBDIR_LIST= platform $(shell ls $(LIBRARY_SOURCE_PATH) --hide=platform --hide=Makefile)
LIBRARY_CONFIG_SUBDIRS = $(foreach f,$(LIBRARY_SUBDIR_LIST),$(if $(BR2_CONFIG_LIBRARY_$(shell echo $f | tr a-z- A-Z_)),$(f),))
LIBRARY_CONFIG_ALL_TARGETS = $(addsuffix inc, $(wildcard $(INC-PATH)))

LIBRARY_SUBDIR_ENV=LIBRARY_INSTALL_PATH=$(LIBRARY_INSTALL_PATH) \
           CROSS_COMPILE=$(TARGET_CROSS) \
           SDK_INSTALL_PATH=$(SDK_PATH) \
		   REDTEA_SUPPORT_SCRIPTS_PATH=$(PWD)/support/redtea \
		   PLATFORM_TYPE=$(SDK_PLATFORM_TYPE) \
		   DEBUG_TYPE=$(SDK_SOFTWARE_TYPE) \
		   SYSROOT=$(TARGET_SYSROOT) \
		   SYSINC=$(TARGET_SYSINC)

SDK_SOFTWARE_NAME=$(call qstrip,$(BR2_SOFTWARE_NAME))
SDK_PLATFORM_TYPE=$(call qstrip,$(BR2_PLATFORM_TYPE))
SDK_SOFTWARE_TYPE=$(call qstrip,$(BR2_SOFTWARE_TYPE))

#$(warning --------PLATFORM_TYPE=$(PLATFORM_TYPE) DEBUG_TYPE=$(SDK_SOFTWARE_TYPE))

LIBRARY_JOIN_LIST=$(patsubst %,library-%,$(LIBRARY_CONFIG_SUBDIRS))
LIBRARY_JOIN_LIST_CLEAN=$(patsubst %,%-clean,$(LIBRARY_JOIN_LIST))
LIBRARY_JOIN_LIST_INSTALL=$(patsubst %,%-install,$(LIBRARY_JOIN_LIST))

define ALONE_LIBRARY_BUILD
	$(Q)test -d $(LIBRARY_DIR)/$(1) || mkdir -p $(LIBRARY_DIR)/$(1)
	$(Q)$(TARGET_MAKE_ENV) $(LIBRARY_SUBDIR_ENV) $(MAKE) -C $(LIBRARY_SOURCE_PATH)/$(1) O=$(LIBRARY_DIR)/$(1) $(2)
endef

define ALONE_LIBRARY_CLEAN
	$(Q)$(TARGET_MAKE_ENV) $(LIBRARY_SUBDIR_ENV) $(MAKE) -C $(LIBRARY_SOURCE_PATH)/$(1) O=$(LIBRARY_DIR)/$(1) $(2)
endef

define ALL_LIBRARY_BUILD
	$(Q)$(TARGET_MAKE_ENV) $(LIBRARY_SUBDIR_ENV) $(MAKE) -C $(LIBRARY_SOURCE_PATH) LIBRARY_ALL_TARGETS="$(1)" O=$(LIBRARY_DIR)
endef

define ALL_LIBRARY_CLEAN
	$(Q)$(TARGET_MAKE_ENV) $(LIBRARY_SUBDIR_ENV) $(MAKE) -C $(LIBRARY_SOURCE_PATH)/$(1) O=$(LIBRARY_DIR)/$(1) $(2)
endef

TEE_IMPL_VERSION ?= $(shell svn info 2>/dev/null | grep '^Revision' | awk '{print $$NF}')
DATE_STR = `date +'%Y-%m-%d %T'`
COMPILER_BY = `whoami`

.library_related_dir:
	@test -d $(LIBRARY_DIR)         || mkdir -p $(LIBRARY_DIR)
	@test -d $(SDK_PATH)/lib        || mkdir -p $(SDK_PATH)/lib
	@test -d $(SDK_PATH)/include    || mkdir -p $(SDK_PATH)/include
#	@test -d $(SDK_PATH)/bin    	|| mkdir -p $(SDK_PATH)/bin
#	@test -d $(SDK_PATH)/doc    	|| mkdir -p $(SDK_PATH)/doc

library: .library_related_dir
	@test -d $(LIBRARY_DIR) || mkdir -p $(LIBRARY_DIR)
	$(foreach d,$(LIBRARY_CONFIG_SUBDIRS),$(call ALONE_LIBRARY_BUILD,$(d),)$(sep))
	$(call ALL_LIBRARY_BUILD,$(LIBRARY_CONFIG_SUBDIRS))

library-clean: $(LIBRARY_JOIN_LIST_CLEAN)
	rm -rf $(LIBRARY_DIR)

$(LIBRARY_JOIN_LIST): .library_related_dir
	$(call ALONE_LIBRARY_BUILD,$(patsubst library-%,%,$@),)

$(LIBRARY_JOIN_LIST_CLEAN):
	rm -rf $(LIBRARY_DIR)/$(patsubst library-%-clean,%,$(@))
	$(call ALONE_LIBRARY_CLEAN,$(patsubst library-%-clean,%,$(@)),clean)

# $(LIBRARY_JOIN_LIST_INSTALL):%-install:%
#	$(call ALONE_LIBRARY_BUILD,$(patsubst library-%-install,%,$(@)),install)

.PHONY: library library-clean

endif
