
ifeq ($(BR2_PACKAGE_OEMAPP_BUILD),y)

TARGETS += oemapp

RELEASE_OEMAPP_INSTALL_PATH=$(call qstrip,$(BR2_RELEASE_OEMAPP_INSTALL_PATH))
RELEASE_OEMAPP_FLASH_OUTPUT_PATH=$(call qstrip,$(BR2_RELEASE_OEMAPP_FLASH_OUTPUT_PATH))
RELEASE_OEMAPP_OTA_OUTPUT_PATH=$(call qstrip,$(BR2_RELEASE_OEMAPP_OTA_OUTPUT_PATH))
RELEASE_OEMAPP_VERSION=$(call qstrip,$(BR2_RELEASE_OEMAPP_VERSION))
RELEASE_OEMAPP_UBI_TARGET=$(call qstrip,$(BR2_RELEASE_OEMAPP_UBI_TARGET))
RELEASE_OEMAPP_SYSTEM_NAME=$(call qstrip,$(BR2_CFG_SYSTEM_NAME))
RELEASE_OEMAPP_PRODUCT_NAME=$(call qstrip,$(BR2_CFG_PRODUCT_NAME))
RELEASE_OEMAPP_SOFTWARE_NAME=$(call qstrip,$(BR2_CFG_SOFTWARE_NAME))
RELEASE_OEMAPP_PLATFORM_TYPE=$(call qstrip,$(BR2_CFG_PLATFORM_TYPE))
RELEASE_OEMAPP_SOFTWARE_TYPE=$(call qstrip,$(BR2_CFG_SOFTWARE_TYPE))
RELEASE_OEMAPP_ENV_TYPE=$(call qstrip,$(BR2_CFG_ENV_TYPE))
RELEASE_OEMAPP_TARGET_NAME=$(RELEASE_OEMAPP_SYSTEM_NAME)-$(RELEASE_OEMAPP_PRODUCT_NAME)-oemapp-$(RELEASE_OEMAPP_SOFTWARE_NAME)

RELEASE_OEMAPP_IMG_FILE=$(RELEASE_OEMAPP_FLASH_OUTPUT_PATH)/oemapp.squashfs
RELEASE_OEMAPP_CFG_FILE=$(RELEASE_OEMAPP_FLASH_OUTPUT_PATH)/ubinize_oemapp_ubi.cfg
RELEASE_OEMAPP_OTA_UBI_FILE=$(RELEASE_OEMAPP_OTA_OUTPUT_PATH)/$(BR2_RELEASE_OEMAPP_UBI_TARGET)
REDTEA_SUPPORT_SCRIPTS_PATH=$(PWD)/support/scripts
RELEASE_OEMAPP_UBI_TOOL=$(REDTEA_SUPPORT_SCRIPTS_PATH)/ubinize_oemapp_ubi.sh
RELEASE_OEMAPP_SIGN_TOOL=$(REDTEA_SUPPORT_SCRIPTS_PATH)/sign_file.sh
REDTEA_OEMAPP_VERSION=$(BR2_RELEASE_OEMAPP_INSTALL_PATH)/oemapp_version
REDTEA_OEMAPP_VERSION_FILE_TITLE="Release: "   # never modify !!!
REDTEA_OEMAPP_VERSION_FILE=$(BR2_RELEASE_OEMAPP_INSTALL_PATH)/softsim-release
REDTEA_OEMAPP_SHELL_START=../doc/shells/standard/start_oemapp.sh
REDTEA_OEMAPP_SHELL_APP=../doc/shells/standard/start_redtea_app
REDTEA_OEMAPP_SHELL_KEEP=../doc/shells/standard/start_redtea_keep
REDTEA_OEMAPP_TOOLS=../doc/tools/
REDTEA_OEMAPP_SHARE_PROFILES=../doc/share_profile/*.der
REDTEA_OEMAPP_SHARE_PROFILE=rt_share_profile.der

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
#                  MFR      agent     monitor    so       ubi       share profile batch code 
# e.g. "Release: general_v4.5.6.10_v7.8.9.10_v1.2.3.10_v12.15.18.30#B191213070351591259"
define CREATE_OEMAPP_SOFTSIM_RELEASE
	if [ true ]; then \
		version_string=$(REDTEA_OEMAPP_VERSION_FILE_TITLE); \
		targets_list=($(notdir $(shell ls $(SYSAPP_INSTALL_PATH)/*agent*)) $(notdir $(shell ls $(SYSAPP_INSTALL_PATH)/*monitor*)) $(notdir $(shell ls $(SDK_PATH)/lib/*-libcomm.so*))); \
		manufacturer_name=`echo $${targets_list[0]} | cut -d "_" -f 1 | cut -d "-" -f 4`; \
		share_profile_batch_code="Bxxxxxxxxxxxxxxxxxx"; \
		if [ -n "`ls $(REDTEA_OEMAPP_SHARE_PROFILES)`" ] ; \
		then \
			share_profile_batch_code=`read -n 100 data < $(REDTEA_OEMAPP_SHARE_PROFILES) ; echo $$data | grep -Eo "B[0-9]{0,18}"`; \
		fi; \
		version_string+="$$manufacturer_name"; \
		ubi_version_1=0; \
		ubi_version_2=0; \
		ubi_version_3=0; \
		ubi_version_4=0; \
		for var in $${targets_list[*]};do \
			app_all_name=`echo "$$var" | cut -d "_" -f 1`; \
			app_name="`echo "$$app_all_name" | cut -d "-" -f 3`"; \
			app_version=`echo "$$var" | cut -d "_" -f 2`; \
			version_string+="_$$app_version"; \
			app_version_x=`echo "$$app_version" | sed 's/v//g'`; \
			tmp=`echo "$$app_version_x" | cut -d "." -f 1`;ubi_version_1=`expr $$ubi_version_1 + $$tmp`; \
			tmp=`echo "$$app_version_x" | cut -d "." -f 2`;ubi_version_2=`expr $$ubi_version_2 + $$tmp`; \
			tmp=`echo "$$app_version_x" | cut -d "." -f 3`;ubi_version_3=`expr $$ubi_version_3 + $$tmp`; \
			tmp=`echo "$$app_version_x" | cut -d "." -f 4`;ubi_version_4=`expr $$ubi_version_4 + $$tmp`; \
		done; \
		ubi_version=v$$ubi_version_1.$$ubi_version_2.$$ubi_version_3.$$ubi_version_4; \
		version_string+="_$$ubi_version""#""$$share_profile_batch_code"; \
		echo "$$version_string" > $(REDTEA_OEMAPP_VERSION_FILE); \
		echo -e "$$ubi_version-$$share_profile_batch_code\c" > $(REDTEA_OEMAPP_VERSION); \
		echo "Oemapp Ubi $$version_string"; \
	fi
endef

# Copy targets into oemapp
define COPY_TEST_OEMAPP_TARGETS
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/test_lpa $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/test_lpa
	-$(Q)if [ -d $(REDTEA_OEMAPP_TOOLS) ] ; \
	then \
		if [ -n "`ls $(REDTEA_OEMAPP_TOOLS)`" ] ; \
		then \
			cp -rf $(REDTEA_OEMAPP_TOOLS)/* $(BR2_RELEASE_OEMAPP_INSTALL_PATH) ; \
		fi; \
	fi;	
endef

# Copy release share profile
define COPY_SHARE_PROFILE
	-$(Q)if [ -n "`ls $(REDTEA_OEMAPP_SHARE_PROFILES)`" ] ; \
	then \
		cp -rf $(REDTEA_OEMAPP_SHARE_PROFILES) $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/$(REDTEA_OEMAPP_SHARE_PROFILE) ; \
	fi;
endef

define COPY_RELEASE_OEMAPP_TARGETS
	-$(Q)cp -rf $(REDTEA_OEMAPP_SHELL_START) $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/
	-$(Q)cp -rf $(REDTEA_OEMAPP_SHELL_APP) $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/
	-$(Q)cp -rf $(REDTEA_OEMAPP_SHELL_KEEP) $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*agent* $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/rt_agent
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*monitor* $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/rt_monitor	
	-$(Q)cp -rf $(SDK_PATH)/lib/*-libcomm.so* $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/libcomm.so
	-$(Q)$(call COPY_SHARE_PROFILE)
	-$(Q)$(call COPY_TEST_OEMAPP_TARGETS)
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
	$(Q)cp -rf $(RELEASE_OEMAPP_OTA_UBI_FILE) $(RELEASE_OEMAPP_OTA_OUTPUT_PATH)/$(notdir $(shell ls $(SYSAPP_INSTALL_PATH)/*agent*))_$(RELEASE_OEMAPP_UBI_TARGET)
	$(Q)mv $(RELEASE_OEMAPP_OTA_UBI_FILE) $(RELEASE_OEMAPP_OTA_OUTPUT_PATH)/$(RELEASE_OEMAPP_TARGET_NAME)_$(shell cat $(REDTEA_OEMAPP_VERSION))_$(RELEASE_OEMAPP_PLATFORM_TYPE)_$(RELEASE_OEMAPP_ENV_TYPE)_$(RELEASE_OEMAPP_SOFTWARE_TYPE)_$(RELEASE_OEMAPP_UBI_TARGET) && rm -rf $(REDTEA_OEMAPP_VERSION)
endef

$(REDTEA_OEMAPP_VERSION):
	@test -e $(RELEASE_OEMAPP_INSTALL_PATH) || mkdir -p $(RELEASE_OEMAPP_INSTALL_PATH)
	@test -e $(RELEASE_OEMAPP_FLASH_OUTPUT_PATH) || mkdir -p $(RELEASE_OEMAPP_FLASH_OUTPUT_PATH)
	@test -e $(RELEASE_OEMAPP_OTA_OUTPUT_PATH) || mkdir -p $(RELEASE_OEMAPP_OTA_OUTPUT_PATH)
	$(Q)$(call CREATE_OEMAPP_CFG_FILE,$(RELEASE_OEMAPP_IMG_FILE),$(RELEASE_OEMAPP_CFG_FILE))
	$(Q)$(call COPY_RELEASE_OEMAPP_TARGETS)
	$(Q)$(call CREATE_OEMAPP_SOFTSIM_RELEASE)	
	
oemapp_ubi: $(REDTEA_OEMAPP_VERSION)
	$(Q)$(call CREATE_OEMAPP_UBI_FILE)
	$(Q)$(call CREATE_OEMAPP_OTA_UBI_FILE)
	
oemapp: $(REDTEA_OEMAPP_VERSION) oemapp_ubi
	
oemapp-clean:
	rm -rf $(RELEASE_INSTALL_PATH)

.PHONY: oemapp oemapp-clean oemapp_ubi

endif