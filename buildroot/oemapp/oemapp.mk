
ifeq ($(BR2_PACKAGE_OEMAPP_BUILD),y)

TARGETS += oemapp

RELEASE_OEMAPP_INSTALL_PATH=$(call qstrip,$(BR2_RELEASE_OEMAPP_INSTALL_PATH))
RELEASE_OEMAPP_FLASH_OUTPUT_PATH=$(call qstrip,$(BR2_RELEASE_OEMAPP_FLASH_OUTPUT_PATH))
RELEASE_OEMAPP_OTA_OUTPUT_PATH=$(call qstrip,$(BR2_RELEASE_OEMAPP_OTA_OUTPUT_PATH))
RELEASE_OEMAPP_VERSION=$(call qstrip,$(BR2_RELEASE_OEMAPP_VERSION))
RELEASE_OEMAPP_UBI_TARGET=$(call qstrip,$(BR2_RELEASE_OEMAPP_UBI_TARGET))

RELEASE_OEMAPP_IMG_FILE=$(RELEASE_OEMAPP_FLASH_OUTPUT_PATH)/oemapp.squashfs
RELEASE_OEMAPP_CFG_FILE=$(RELEASE_OEMAPP_FLASH_OUTPUT_PATH)/ubinize_oemapp_ubi.cfg
RELEASE_OEMAPP_OTA_UBI_FILE=$(RELEASE_OEMAPP_OTA_OUTPUT_PATH)/$(BR2_RELEASE_OEMAPP_UBI_TARGET)
REDTEA_SUPPORT_SCRIPTS_PATH=$(PWD)/support/scripts
RELEASE_OEMAPP_UBI_TOOL=$(REDTEA_SUPPORT_SCRIPTS_PATH)/ubinize_oemapp_ubi.sh
RELEASE_OEMAPP_SIGN_TOOL=$(REDTEA_SUPPORT_SCRIPTS_PATH)/sign_file.sh
REDTEA_OEMAPP_VERSION_FILE_TITLE="Release: "   # never modify !!!
REDTEA_OEMAPP_VERSION_FILE=$(BR2_RELEASE_OEMAPP_INSTALL_PATH)/softsim-release
REDTEA_SHELL_START_OEMAPP=../doc/shells/start_oemapp.sh

# Auto generate oemapp cfg file
define CREATE_OEMAPP_CFG_FILE
	echo "" >$(2);\
    echo "[appromfs_volume]" >$(2);\
    echo "mode=ubi" >>$(2);\
	echo "image=$(1)" >>$(2);\
	echo "vol_id=0" >>$(2);\
	echo "vol_type=dynamic" >>$(2);\
	echo "vol_name=appromfs" >>$(2);\
	echo "vol_flags=autoresize" >>$(2);\
	echo "" >>$(2)
endef

# Auto generate softsim-release file
define CREATE_OEMAPP_SOFTSIM_RELEASE
	if [ true ]; then\
	version_string=$(REDTEA_OEMAPP_VERSION_FILE_TITLE);\
	targets_list="$(notdir $(shell ls $(SYSAPP_INSTALL_PATH)/*agent*)) $(notdir $(shell ls $(SYSAPP_INSTALL_PATH)/*monitor*)) $(notdir $(shell ls $(SDK_PATH)/lib/*-libcomm.so*)) ";\
	for var in $$targets_list;do \
	version_string+="[`echo "$$var" | cut -d "_" -f 1`_`echo "$$var" | cut -d "_" -f 2`] "; done;\
	echo "$$version_string" > $(REDTEA_OEMAPP_VERSION_FILE);\
	fi
endef

define COPY_RELEASE_OEMAPP_TARGETS
	-$(Q)cp -rf $(REDTEA_SHELL_START_OEMAPP) $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*agent* $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/rt_agent
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*monitor* $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/rt_monitor	
	-$(Q)cp -rf $(SDK_PATH)/lib/*-libcomm.so* $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/libcomm.so
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/test_lpa $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/test_lpa
endef

define CREATE_OEMAPP_UBI_FILE
	$(Q)$(RELEASE_OEMAPP_UBI_TOOL) $(RELEASE_OEMAPP_INSTALL_PATH) $(RELEASE_OEMAPP_FLASH_OUTPUT_PATH) $(dir $(RELEASE_OEMAPP_UBI_TOOL)) $(RELEASE_OEMAPP_UBI_TARGET) $(RELEASE_OEMAPP_VERSION)
endef

# Add SHA256withECC signature to the tail of a file
define UBI_ADD_SHA256withECC
	$(RELEASE_OEMAPP_SIGN_TOOL) $(1) $(REDTEA_SUPPORT_SCRIPTS_PATH) Q=$(Q)
endef

define CREATE_OEMAPP_OTA_UBI_FILE
	$(Q)cp -rf $(RELEASE_OEMAPP_FLASH_OUTPUT_PATH)/$(RELEASE_OEMAPP_UBI_TARGET) $(RELEASE_OEMAPP_OTA_OUTPUT_PATH)/
	$(Q)$(call UBI_ADD_SHA256withECC,$(RELEASE_OEMAPP_OTA_UBI_FILE))
	$(Q)mv $(RELEASE_OEMAPP_OTA_UBI_FILE) $(RELEASE_OEMAPP_OTA_OUTPUT_PATH)/$(notdir $(shell ls $(SYSAPP_INSTALL_PATH)/*agent*))_$(RELEASE_OEMAPP_UBI_TARGET)
endef

oemapp:
	@test -e $(RELEASE_OEMAPP_INSTALL_PATH) || mkdir -p $(RELEASE_OEMAPP_INSTALL_PATH)
	@test -e $(RELEASE_OEMAPP_FLASH_OUTPUT_PATH) || mkdir -p $(RELEASE_OEMAPP_FLASH_OUTPUT_PATH)
	@test -e $(RELEASE_OEMAPP_OTA_OUTPUT_PATH) || mkdir -p $(RELEASE_OEMAPP_OTA_OUTPUT_PATH)
	$(Q)$(call CREATE_OEMAPP_CFG_FILE,$(RELEASE_OEMAPP_IMG_FILE),$(RELEASE_OEMAPP_CFG_FILE))
	$(Q)$(call COPY_RELEASE_OEMAPP_TARGETS)
	$(Q)$(call CREATE_OEMAPP_SOFTSIM_RELEASE)	
	$(Q)$(call CREATE_OEMAPP_UBI_FILE)
	$(Q)$(call CREATE_OEMAPP_OTA_UBI_FILE)
	
oemapp-clean:
	rm -rf $(RELEASE_INSTALL_PATH)

.PHONY: oemapp oemapp-clean

endif