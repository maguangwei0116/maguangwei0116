
TARGETS += sysapp

ifeq ($(BR2_PACKAGE_SYSAPP),y)

SYSAPP_DIR=$(BUILD_DIR)/sysapp
SYSAPP_SOURCE_PATH=$(call qstrip,$(BR2_SYSAPP_SOURCE_PATH))
SYSAPP_INSTALL_PATH=$(call qstrip,$(BR2_SYSAPP_INSTALL_PATH))
SYSAPP_SUBDIR_LIST=$(shell ls $(SYSAPP_SOURCE_PATH))
SYSAPP_CONFIG_SUBDIRS = $(foreach f,$(SYSAPP_SUBDIR_LIST),$(if $(BR2_CONFIG_SYSAPP_$(shell echo $f | tr a-z- A-Z_)),$(f),))
SYSAPP_SYSTEM_NAME=$(call qstrip,$(BR2_SYSTEM_NAME))
SYSAPP_PRODUCT_NAME=$(call qstrip,$(BR2_PRODUCT_NAME))
SYSAPP_SOFTWARE_NAME=$(call qstrip,$(BR2_SOFTWARE_NAME))
SYSAPP_PLATFORM_TYPE=$(call qstrip,$(BR2_PLATFORM_TYPE))
SYSAPP_SOFTWARE_TYPE=$(call qstrip,$(BR2_SOFTWARE_TYPE))
SYSAPP_ENV_TYPE=$(call qstrip,$(BR2_CFG_ENV_TYPE))

# Get git head commit hash string (frist 8 chars)
SYSAPP_GIT_PATH=$(SYSAPP_SOURCE_PATH)/../.git/
GIT_PATH=$(SYSAPP_GIT_PATH)
GIT_HEAD_FILE=$(GIT_PATH)/HEAD
GIT_HASH_NUM_CNT=8
ifeq ($(GIT_HEAD_FILE), $(wildcard $(GIT_HEAD_FILE)))
SYSAPP_GIT_HASH=$(shell echo $(GIT_PATH)/$(shell cat $(GIT_HEAD_FILE) | awk '{ print $$2}') | xargs cat | cut -c1-$(GIT_HASH_NUM_CNT))
else
SYSAPP_GIT_HASH=xxxxxxxx
endif

SYSAPP_SUBDIR_ENV=SYSAPP_INSTALL_PATH=$(SYSAPP_INSTALL_PATH) \
           CROSS_COMPILE=$(TARGET_CROSS) \
           SDK_INSTALL_PATH=$(SDK_PATH) \
		   REDTEA_SUPPORT_SCRIPTS_PATH=$(PWD)/support/redtea \
		   SYSAPP_SOFTWARE_NAME=$(SYSAPP_SOFTWARE_NAME) \
		   PLATFORM_TYPE=$(SYSAPP_PLATFORM_TYPE) \
		   DEBUG_TYPE=$(SYSAPP_SOFTWARE_TYPE) \
		   SYSROOT=$(TARGET_SYSROOT) \
		   SYSINC=$(TARGET_SYSINC) \
		   SYSAPP_GIT_HASH=$(SYSAPP_GIT_HASH) \
		   KCONFIG_AUTOHEADER=$(KCONFIG_AUTOHEADER_FILE) \
		   SYSAPP_ENV_TYPE=$(SYSAPP_ENV_TYPE)

SYSAPP_JOIN_LIST=$(patsubst %,sysapp-%,$(SYSAPP_CONFIG_SUBDIRS))
SYSAPP_JOIN_LIST_CLEAN=$(patsubst %,%-clean,$(SYSAPP_JOIN_LIST))
SYSAPP_JOIN_LIST_INSTALL=$(patsubst %,%-install,$(SYSAPP_JOIN_LIST))

OUTPUT_BR2_CONF_MK=./include/generated/conf.mk

define ECHO_BR2_CONF_MK_ITEM
	echo "$(1)=$(2)" >> $(3);
endef

define AUTO_GEN_BR2_CONF_MK
	$(foreach var,$(1),$(call ECHO_BR2_CONF_MK_ITEM,$(var),$($(var)),$(2)))
	@sed -i 's/BR2_//' $(2)
endef

define AUTO_GEN_BR2_CFG_VARS
	@test -e $(dir $(2)) || mkdir -p $(dir $(2))
	@echo > $(2)
	@$(call AUTO_GEN_BR2_CONF_MK,$(foreach var,$(1),$(filter $(var)%,$(.VARIABLES))),$(2))
	@echo >> $(2)
endef

define ALONE_SYSAPP_BUILD
	@test -d $(SYSAPP_DIR)/$(1) || mkdir -p $(SYSAPP_DIR)/$(1)
	$(Q)$(call AUTO_GEN_BR2_CFG_VARS,BR2_CFG,$(SYSAPP_DIR)/$(1)/$(OUTPUT_BR2_CONF_MK))
	$(Q)$(TARGET_MAKE_ENV) $(SYSAPP_SUBDIR_ENV) $(MAKE) -C $(SYSAPP_SOURCE_PATH)/$(1) BR2_CONF_MK=$(SYSAPP_DIR)/$(1)/$(OUTPUT_BR2_CONF_MK) SYSAPP_TARGET_NAME=$(SYSAPP_SYSTEM_NAME)-$(SYSAPP_PRODUCT_NAME)-$(1)-$(SYSAPP_SOFTWARE_NAME) O=$(SYSAPP_DIR)/$(1) $(2)
endef

define ALONE_SYSAPP_CLEAN
	$(Q)$(TARGET_MAKE_ENV) $(SYSAPP_SUBDIR_ENV) $(MAKE) -C $(SYSAPP_SOURCE_PATH)/$(1) SYSAPP_TARGET_NAME=$(SYSAPP_SYSTEM_NAME)-$(SYSAPP_PRODUCT_NAME)-$(1)-$(SYSAPP_SOFTWARE_NAME) O=$(SYSAPP_DIR)/$(1) $(2)
endef

.sysapp_related_dir:
	@test -d $(SYSAPP_INSTALL_PATH) || mkdir -p $(SYSAPP_INSTALL_PATH)
	@test -d $(SYSAPP_DIR) || mkdir -p $(SYSAPP_DIR)

sysapp: .sysapp_related_dir
	@test -d $(SYSAPP_DIR) || mkdir -p $(SYSAPP_DIR)
	$(foreach d,$(SYSAPP_CONFIG_SUBDIRS),$(call ALONE_SYSAPP_BUILD,$(d),)$(sep))
	$(Q)echo "all sysapp build done !"

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
