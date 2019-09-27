ifeq ($(BR2_PACKAGE_RELEASE_BUILD),y)

TARGETS += release

RELEASE_INSTALL_PATH=$(call qstrip,$(BR2_RELEASE_INSTALL_PATH))
RELEASE_BUILD_DIR=$(BUILD_DIR)/release

RELEASE_VERSION=$(call qstrip,$(BR2_RELEASE_VERSION))
#$(warning BR2_RELEASE_TARGET=$(BR2_RELEASE_TARGET))
RELEASE_TARGET=$(call qstrip,$(BR2_RELEASE_TARGET))
RELEASE_ENV_TYPE=$(call qstrip,$(BR2_CFG_ENV_TYPE))

REDTEA_RELEASE_README=README.txt
REDTEA_CHANGE_LOG=../doc/change-log.txt
REDTEA_SHELL_APP=../doc/shells/start_redtea_app
REDTEA_SHELL_KEEP=../doc/shells/start_redtea_keep
REDTEA_ADB_PUSH_SHELL=adb-push.sh
REDTEA_SHELL_ADB_PUSH=../doc/shells/$(REDTEA_ADB_PUSH_SHELL)
RELEASE_AUTHOR=$(shell whoami)
RELEASE_DATE=$(shell date +"%Y%m%d")
RELEASE_TIME=$(shell date +"%Y-%m-%d %T")
RELEASE_TAR_FILE=$(RELEASE_TARGET)-$(RELEASE_ENV_TYPE)-targets-$(RELEASE_VERSION)-$(RELEASE_DATE).zip

# Auto generate README.txt
define CREATE_RELEASE_README
	echo "" >$(1);\
    echo "/* Auto-generated readme for released targets ! */" >$(1);\
    echo "" >>$(1);\
    echo "Author      : ${RELEASE_AUTHOR}" >>$(1);\
    echo "Time        : ${RELEASE_TIME}" >>$(1);\
	echo "Version     : ${RELEASE_VERSION}" >>$(1);\
	echo "Target      : ${RELEASE_TARGET}" >>$(1);\
	echo "Environment : ${RELEASE_ENV_TYPE}" >>$(1);\
	echo "" >>$(1);\
	echo "Tips : " >>$(1);\
	echo "1.Copy $(RELEASE_TAR_FILE) into your debug computer;" >>$(1);\
	echo "2.Unzip $(RELEASE_TAR_FILE) with cmd [ unzip $(RELEASE_TAR_FILE) ];" >>$(1);\
	echo "3.Read doc in ./$(shell basename $(RELEASE_INSTALL_PATH))/$(REDTEA_RELEASE_README) which you are reading;" >>$(1);\
	echo "4.Connect your target device with USB adb;" >>$(1);\
	echo "5.Enter ./$(shell basename $(RELEASE_INSTALL_PATH)) with cmd [ cd ./$(shell basename $(RELEASE_INSTALL_PATH)) ];" >>$(1);\
	echo "6.Run [ ./$(REDTEA_ADB_PUSH_SHELL) ] which located in ./$(shell basename $(RELEASE_INSTALL_PATH)) to push all targes into target device with adb;" >>$(1);\
	echo "7.Reboot target device to autorun new target applications." >>$(1);\
	echo "" >>$(1)
endef

define COPY_RELEASE_TARGETS
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*agent* $(RELEASE_INSTALL_PATH)/targets
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*monitor* $(RELEASE_INSTALL_PATH)/targets
	-$(Q)cp -rf $(SDK_PATH)/lib/*libcomm.so $(RELEASE_INSTALL_PATH)/targets
	-$(Q)cp -rf $(REDTEA_SHELL_APP) $(RELEASE_INSTALL_PATH)/shells
	-$(Q)cp -rf $(REDTEA_SHELL_KEEP) $(RELEASE_INSTALL_PATH)/shells
	-$(Q)cp -rf $(REDTEA_SHELL_ADB_PUSH) $(RELEASE_INSTALL_PATH)
	-$(Q)cp -rf $(REDTEA_CHANGE_LOG) $(RELEASE_INSTALL_PATH)
endef

# Tar release targets
define TAR_RELEASE_TARGETS
	cd $(1)/../; rm -rf $(1)/*.zip; zip -q -r $(2) ./$(shell basename $(1)); mv $(2) $(1) 
endef

release:
	@test -e $(RELEASE_INSTALL_PATH)/targets || mkdir -p $(RELEASE_INSTALL_PATH)/targets
	@test -e $(RELEASE_INSTALL_PATH)/shells || mkdir -p $(RELEASE_INSTALL_PATH)/shells
	$(Q)$(call COPY_RELEASE_TARGETS)
	$(Q)test -e $(RELEASE_INSTALL_PATH)/$(REDTEA_RELEASE_README) || $(call CREATE_RELEASE_README,$(RELEASE_INSTALL_PATH)/$(REDTEA_RELEASE_README))
	$(Q)$(call TAR_RELEASE_TARGETS,$(RELEASE_INSTALL_PATH),$(RELEASE_TAR_FILE))
	
release-clean:
	rm -rf $(RELEASE_INSTALL_PATH) $(RELEASE_BUILD_DIR)

.PHONY: release release-clean

endif