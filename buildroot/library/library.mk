
TARGETS += library

ifeq ($(BR2_PACKAGE_LIBRARY),y)

LIBRARY_DIR=$(BUILD_DIR)/library
LIBRARY_SOURCE_PATH=$(call qstrip,$(BR2_LIBRARY_SOURCE_PATH))
LIBRARY_INSTALL_PATH=$(call qstrip,$(BR2_LIBRARY_INSTALL_PATH))
LIBRARY_SUBDIR_LIST= platform $(shell ls $(LIBRARY_SOURCE_PATH) --hide=platform)
LIBRARY_CONFIG_SUBDIRS = $(foreach f,$(LIBRARY_SUBDIR_LIST),$(if $(BR2_CONFIG_LIBRARY_$(shell echo $f | tr a-z- A-Z_)),$(f),))

LIBRARY_SUBDIR_ENV=SYSAPP_INSTALL_PATH=$(SYSAPP_INSTALL_PATH) \
		   LIBRARY_INSTALL_PATH=$(LIBRARY_INSTALL_PATH) \
           SYSAPP_INSTALL_PATH=$(SYSAPP_INSTALL_PATH) \
           FIRMWARE_INSTALL_PATH=$(FIRMWARE_INSTALL_PATH) \
           TOOLS_INSTALL_PATH=$(TOOLS_INSTALL_PATH) \
           CROSS_COMPILE=$(TARGET_CROSS) \
           SDK_INSTALL_PATH=$(SDK_PATH) \
		   SYSTEM_PATH=$(SYSTEM_DIR) \
		   FIRMWARE_PCI_COMPLY=$(FIRMWARE_PCI_COMPLY) \
		   FIRMWARE_UPTS_COMPLY=$(FIRMWARE_UPTS_COMPLY) \
		   PLATFORM_TYPE=$(SDK_PLATFORM_TYPE) \
		   DEBUG_TYPE=$(SDK_FIRMWARE_TYPE) \
		   SYSROOT=$(TARGET_SYSROOT) \
		   SYSINC=$(TARGET_SYSINC)


SDK_VERSION=$(call qstrip,$(BR2_IMAGE_VERSION))
SDK_PRODUCT_NAME=$(call qstrip,$(BR2_PRODUCT_NAME))
SDK_PLATFORM_TYPE=$(call qstrip,$(BR2_PLATFORM_TYPE))
SDK_FIRMWARE_TYPE=$(call qstrip,$(BR2_FIRMWARE_TYPE))

#$(warning --------PLATFORM_TYPE=$(PLATFORM_TYPE) DEBUG_TYPE=$(SDK_FIRMWARE_TYPE))

LIBRARY_JOIN_LIST=$(patsubst %,library-%,$(LIBRARY_CONFIG_SUBDIRS))
LIBRARY_JOIN_LIST_CLEAN=$(patsubst %,%-clean,$(LIBRARY_JOIN_LIST))
LIBRARY_JOIN_LIST_INSTALL=$(patsubst %,%-install,$(LIBRARY_JOIN_LIST))

define ALONE_LIBRARY_BUILD
	$(Q)test -d $(LIBRARY_DIR)/$(1) || mkdir -p $(LIBRARY_DIR)/$(1)
	$(Q)$(TARGET_MAKE_ENV) $(LIBRARY_SUBDIR_ENV) $(MAKE) -C $(LIBRARY_SOURCE_PATH)/$(1) NDK_PATH=$(LIB_NDK_PATH) O=$(LIBRARY_DIR)/$(1) $(2)
endef

define ALONE_LIBRARY_CLEAN
	$(Q)$(TARGET_MAKE_ENV) $(LIBRARY_SUBDIR_ENV) $(MAKE) -C $(LIBRARY_SOURCE_PATH)/$(1) NDK_PATH=$(LIB_NDK_PATH) O=$(LIBRARY_DIR)/$(1) $(2)
endef

TEE_IMPL_VERSION ?= $(shell svn info 2>/dev/null | grep '^Revision' | awk '{print $$NF}')
DATE_STR = `date +'%Y-%m-%d %T'`
COMPILER_BY = `whoami`

define SDK_LIBRARY_VERSION_BUILD
	$(Q)echo "SDK-Library: [$(SDK_PRODUCT_NAME)-$(SDK_PLATFORM_TYPE)-$(SDK_FIRMWARE_TYPE)] svn-$(TEE_IMPL_VERSION) v$(SDK_VERSION) compiled by \""$(COMPILER_BY)\"" @ $(DATE_STR)" > $(SDK_PATH)/sdk-version.txt
endef

PACKED_DATE = `date +'%Y%m%d'`
define SDK_LIBRARY_PACKED
	$(Q)$(TAR) -cjf $(SDK_PATH)/../sdk-$(SDK_PRODUCT_NAME)-$(SDK_PLATFORM_TYPE)-$(SDK_FIRMWARE_TYPE)-$(PACKED_DATE)-svn$(TEE_IMPL_VERSION)-v$(SDK_VERSION).tar.bz2 $(SDK_PATH)/../sdk/ >/dev/null 2>&1
endef

define LIBRARY_EMV_COPYED
	$(Q)if [ -x $(LIBRARY_SOURCE_PATH)/emv/libemv.a ]; then \
		cp $(LIBRARY_SOURCE_PATH)/emv/libemv.a $(SDK_PATH)/lib/ ;\
	fi
	$(Q)if [ -n "`ls $(LIBRARY_SOURCE_PATH)/emv/include/*.h`" ]; then \
		cp $(LIBRARY_SOURCE_PATH)/emv/include/*.h $(SDK_PATH)/include/ ;\
	fi
endef

define SDK_DOC_COPYED
	$(Q)if [ -x ../doc/sdk-doc/N58G-Application-Development-Guide.doc ]; then \
		cp ../doc/sdk-doc/N58G-Application-Development-Guide.doc $(SDK_PATH)/doc/ ;\
	fi
endef

.library_related_dir:
	@test -d $(SYSAPP_INSTALL_PATH) || mkdir -p $(SYSAPP_INSTALL_PATH)
	@test -d $(LIBRARY_DIR)         || mkdir -p $(LIBRARY_DIR)
	@test -d $(SDK_PATH)/lib        || mkdir -p $(SDK_PATH)/lib
	@test -d $(SDK_PATH)/include    || mkdir -p $(SDK_PATH)/include
#	@test -d $(SDK_PATH)/bin    	|| mkdir -p $(SDK_PATH)/bin
#	@test -d $(SDK_PATH)/doc    	|| mkdir -p $(SDK_PATH)/doc

library: .library_related_dir
	@test -d $(LIBRARY_DIR) || mkdir -p $(LIBRARY_DIR)
	$(foreach d,$(LIBRARY_CONFIG_SUBDIRS),$(call ALONE_LIBRARY_BUILD,$(d),)$(sep))
#	$(call SDK_DOC_COPYED)
#	$(call LIBRARY_EMV_COPYED)
#	$(call SDK_LIBRARY_VERSION_BUILD)
#	$(call SDK_LIBRARY_PACKED)

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
