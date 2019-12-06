
ifeq ($(BR2_PACKAGE_OEMAPP_BUILD),y)

TARGETS += oemapp

RELEASE_OEMAPP_INSTALL_PATH=$(call qstrip,$(BR2_RELEASE_OEMAPP_INSTALL_PATH))
RELEASE_OEMAPP_OUTPUT_PATH=$(call qstrip,$(BR2_RELEASE_OEMAPP_OUTPUT_PATH))
RELEASE_OEMAPP_VERSION=$(call qstrip,$(BR2_RELEASE_OEMAPP_VERSION))
RELEASE_OEMAPP_UBI_TARGET=$(call qstrip,$(BR2_RELEASE_OEMAPP_UBI_TARGET))

RELEASE_OEMAPP_IMG_FILE=$(RELEASE_OEMAPP_OUTPUT_PATH)/oemapp.squashfs
RELEASE_OEMAPP_CFG_FILE=$(RELEASE_OEMAPP_OUTPUT_PATH)/ubinize_oemapp_ubi.cfg
REDTEA_SUPPORT_SCRIPTS_PATH=$(PWD)/support/scripts
RELEASE_OEMAPP_UBI_TOOL=$(REDTEA_SUPPORT_SCRIPTS_PATH)/ubinize_oemapp_ubi.sh

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

define COPY_RELEASE_OEMAPP_TARGETS
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*agent* $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/rt_agent
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*monitor* $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/rt_monitor	
	-$(Q)cp -rf $(SDK_PATH)/lib/*-libcomm.so* $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/libcomm.so
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/test_lpa $(BR2_RELEASE_OEMAPP_INSTALL_PATH)/test_lpa
endef

define CREATE_OEMAPP_UBI_FILE
	-$(Q)$(RELEASE_OEMAPP_UBI_TOOL) $(RELEASE_OEMAPP_INSTALL_PATH) $(RELEASE_OEMAPP_OUTPUT_PATH) $(dir $(RELEASE_OEMAPP_UBI_TOOL)) $(RELEASE_OEMAPP_UBI_TARGET) $(RELEASE_OEMAPP_VERSION)
endef

oemapp:
	@test -e $(RELEASE_OEMAPP_INSTALL_PATH) || mkdir -p $(RELEASE_OEMAPP_INSTALL_PATH)
	@test -e $(RELEASE_OEMAPP_OUTPUT_PATH) || mkdir -p $(RELEASE_OEMAPP_OUTPUT_PATH)
	$(Q)$(call CREATE_OEMAPP_CFG_FILE,$(RELEASE_OEMAPP_IMG_FILE),$(RELEASE_OEMAPP_CFG_FILE))
	$(Q)$(call COPY_RELEASE_OEMAPP_TARGETS)
	$(Q)$(CREATE_OEMAPP_UBI_FILE)
	
oemapp-clean:
	rm -rf $(RELEASE_INSTALL_PATH)

.PHONY: oemapp oemapp-clean

endif