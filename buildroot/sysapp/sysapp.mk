
TARGETS += sysapp

ifeq ($(BR2_PACKAGE_SYSAPP),y)

SYSAPP_DIR=$(BUILD_DIR)/sysapp
SYSAPP_SOURCE_PATH=$(call qstrip,$(BR2_SYSAPP_SOURCE_PATH))
SYSAPP_INSTALL_PATH=$(call qstrip,$(BR2_SYSAPP_INSTALL_PATH))
SYSAPP_SUBDIR_LIST=$(shell ls $(SYSAPP_SOURCE_PATH))
SYSAPP_CONFIG_SUBDIRS = $(foreach f,$(SYSAPP_SUBDIR_LIST),$(if $(BR2_CONFIG_SYSAPP_$(shell echo $f | tr a-z- A-Z_)),$(f),))

SYS_PRODUCT_NAME=$(call qstrip,$(BR2_PRODUCT_NAME))
SYS_PLATFORM_TYPE=$(call qstrip,$(BR2_PLATFORM_TYPE))
SYS_FIRMWARE_TYPE=$(call qstrip,$(BR2_FIRMWARE_TYPE))

SYSAPP_SUBDIR_ENV=SYSAPP_INSTALL_PATH=$(SYSAPP_INSTALL_PATH) \
           CROSS_COMPILE=$(TARGET_CROSS) \
           SDK_INSTALL_PATH=$(SDK_PATH) \
		   SUPPORT_SCRIPTS_PATH=$(PWD)/support \
		   SYS_PRODUCT_NAME=$(SYS_PRODUCT_NAME) \
		   PLATFORM_TYPE=$(SYS_PLATFORM_TYPE) \
		   DEBUG_TYPE=$(SYS_FIRMWARE_TYPE) \
		   SYSROOT=$(TARGET_SYSROOT)

SYSAPP_JOIN_LIST=$(patsubst %,sysapp-%,$(SYSAPP_CONFIG_SUBDIRS))
SYSAPP_JOIN_LIST_CLEAN=$(patsubst %,%-clean,$(SYSAPP_JOIN_LIST))
SYSAPP_JOIN_LIST_INSTALL=$(patsubst %,%-install,$(SYSAPP_JOIN_LIST))
SYSAPP_JOIN_LIST_DOWNLOAD=$(patsubst %,%-download,$(SYSAPP_JOIN_LIST))

define ALONE_SYSAPP_BUILD
	@test -d $(SYSAPP_DIR)/$(1) || mkdir -p $(SYSAPP_DIR)/$(1)
	@$(TARGET_MAKE_ENV) $(SYSAPP_SUBDIR_ENV) $(MAKE) -C $(SYSAPP_SOURCE_PATH)/$(1) O=$(SYSAPP_DIR)/$(1) $(2)
endef

define ALONE_SYSAPP_CLEAN
	@$(TARGET_MAKE_ENV) $(SYSAPP_SUBDIR_ENV) $(MAKE) -C $(SYSAPP_SOURCE_PATH)/$(1) O=$(SYSAPP_DIR)/$(1) $(2)
endef

.sysapp_related_dir:
	@test -d $(SYSAPP_INSTALL_PATH) || mkdir -p $(SYSAPP_INSTALL_PATH)
	@test -d $(SYSAPP_DIR) || mkdir -p $(SYSAPP_DIR)

sysapp: .sysapp_related_dir
	@test -d $(SYSAPP_DIR) || mkdir -p $(SYSAPP_DIR)
	$(foreach d,$(SYSAPP_CONFIG_SUBDIRS),$(call ALONE_SYSAPP_BUILD,$(d),)$(sep))
	@echo "all sysapp build done !"

sysapp-clean: $(SYSAPP_JOIN_LIST_CLEAN)
	rm -rf $(SYSAPP_DIR)

$(SYSAPP_JOIN_LIST): .sysapp_related_dir
	$(call ALONE_SYSAPP_BUILD,$(patsubst sysapp-%,%,$@),)

$(SYSAPP_JOIN_LIST_CLEAN):
	rm -rf $(SYSAPP_DIR)/$(patsubst sysapp-%-clean,%,$(@))
	$(call ALONE_SYSAPP_CLEAN,$(patsubst sysapp-%-clean,%,$(@)),clean)

# $(SYSAPP_JOIN_LIST_INSTALL):%-install:%
#	$(call ALONE_SYSAPP_BUILD,$(patsubst sysapp-%-install,%,$(@)),install)

.PHONY: sysapp sysapp-clean

endif
