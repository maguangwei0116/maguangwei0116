ifeq ($(BR2_PACKAGE_RELEASE_BUILD),y)

TARGETS += release

RELEASE_INSTALL_PATH=$(call qstrip,$(BR2_RELEASE_INSTALL_PATH))
RELEASE_BUILD_DIR=$(BUILD_DIR)/release

RELEASE_VERSION=$(call qstrip,$(BR2_RELEASE_VERSION))
#$(warning BR2_RELEASE_TARGET=$(BR2_RELEASE_TARGET))
RELEASE_TARGET=$(call qstrip,$(BR2_RELEASE_TARGET))
RELEASE_ENV_TYPE=$(call qstrip,$(BR2_CFG_ENV_TYPE))

RELEASE_README=README.txt
CHANGE_LOG=../doc/change-log.txt
ADB_PUSH_SHELL=adb-push.sh
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
	echo "3.Read doc in ./$(shell basename $(RELEASE_INSTALL_PATH))/$(RELEASE_README) which you are reading;" >>$(1);\
	echo "4.Connect your target device with USB adb;" >>$(1);\
	echo "5.Enter ./$(shell basename $(RELEASE_INSTALL_PATH)) with cmd [ cd ./$(shell basename $(RELEASE_INSTALL_PATH)) ];" >>$(1);\
	echo "6.Run [ ./$(ADB_PUSH_SHELL) ] which located in ./$(shell basename $(RELEASE_INSTALL_PATH)) to push all targes into target device with adb;" >>$(1);\
	echo "7.Reboot target device to autorun new target applications." >>$(1);\
    echo "" >>$(1)
endef

define CREATE_ADB_PUSH_SHELL
	echo "" >$(1);\
    echo "###### Auto-generated adb push shell ######" >$(1);\
    echo "" >>$(1);\
	echo "#!/bin/bash" >>$(1);\
    echo "adb push ./targets/*agent* /usr/bin/agent" >>$(1);\
	echo "adb shell chmod + /usr/bin/agent" >>$(1);\
	echo "adb push ./targets/*monitor* /usr/bin/monitor" >>$(1);\
	echo "adb shell chmod + /usr/bin/monitor" >>$(1);\
	echo "adb push ./targets/*libcomm.so* /usr/lib/libcomm.so" >>$(1);\
	echo "adb shell chmod + /usr/lib/libcomm.so" >>$(1);\
    echo "" >>$(1);\
	chmod +x $(1)
endef

define COPY_RELEASE_TARGETS
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*agent* $(RELEASE_INSTALL_PATH)/targets
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*monitor* $(RELEASE_INSTALL_PATH)/targets
	-$(Q)cp -rf $(SDK_PATH)/lib/*libcomm.so $(RELEASE_INSTALL_PATH)/targets
	-$(Q)cp -rf $(CHANGE_LOG) $(RELEASE_INSTALL_PATH)
endef

# Tar release targets
define TAR_RELEASE_TARGETS
	cd $(1)/../; rm -rf $(1)/*.zip; zip -q -r $(2) ./$(shell basename $(1)); mv $(2) $(1) 
endef

release:
	@test -e $(RELEASE_INSTALL_PATH)/targets || mkdir -p $(RELEASE_INSTALL_PATH)/targets
	$(Q)$(call COPY_RELEASE_TARGETS)
	$(Q)test -e $(RELEASE_INSTALL_PATH)/$(RELEASE_README) || $(call CREATE_RELEASE_README,$(RELEASE_INSTALL_PATH)/$(RELEASE_README))
	$(Q)test -e $(RELEASE_INSTALL_PATH)/$(ADB_PUSH_SHELL) || $(call CREATE_ADB_PUSH_SHELL,$(RELEASE_INSTALL_PATH)/$(ADB_PUSH_SHELL))
	$(Q)$(call TAR_RELEASE_TARGETS,$(RELEASE_INSTALL_PATH),$(RELEASE_TAR_FILE))
	
	
release-clean:
	rm -rf $(RELEASE_INSTALL_PATH) $(RELEASE_BUILD_DIR)

.PHONY: release release-clean

endif