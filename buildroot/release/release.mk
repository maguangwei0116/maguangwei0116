ifeq ($(BR2_PACKAGE_RELEASE_BUILD),y)

TARGETS += release

RELEASE_INSTALL_PATH=$(call qstrip,$(BR2_RELEASE_INSTALL_PATH))
RELEASE_STANDARD_MODULE=$(call qstrip,$(BR2_CFG_STANDARD_MODULE))
ifeq ($(RELEASE_STANDARD_MODULE),)
RELEASE_STANDARD_MODULE=n
endif
RELEASE_BUILD_DIR=$(BUILD_DIR)/release

RELEASE_VERSION=$(call qstrip,$(BR2_RELEASE_VERSION))
#$(warning BR2_RELEASE_TARGET=$(BR2_RELEASE_TARGET))
RELEASE_TARGET=$(call qstrip,$(BR2_RELEASE_TARGET))
RELEASE_ENV_TYPE=$(call qstrip,$(BR2_CFG_ENV_TYPE))

REDTEA_RELEASE_README=README.txt
REDTEA_CHANGE_LOG=../doc/change-log.txt
REDTEA_SHELL_APP=../doc/shells/open/start_redtea_app
REDTEA_SHELL_KEEP=../doc/shells/open/start_redtea_keep
REDTEA_OEMAPP_SKB_SO=../sysapp/monitor/vuicc/lib/libskb.so
REDTEA_ADB_PUSH_SHELL=adb-push.sh
REDTEA_SHELL_ADB_PUSH=../doc/shells/open/$(REDTEA_ADB_PUSH_SHELL)
RELEASE_AUTHOR=$(shell whoami)
RELEASE_DATE=$(shell date +"%Y%m%d")
RELEASE_TIME=$(shell date +"%Y-%m-%d %T")
ifeq ($(RELEASE_STANDARD_MODULE),y)
RELEASE_TAR_FILE=$(RELEASE_TARGET)-$(RELEASE_ENV_TYPE)-targets-$(RELEASE_VERSION)-standard-$(RELEASE_DATE).zip
else
RELEASE_TAR_FILE=$(RELEASE_TARGET)-$(RELEASE_ENV_TYPE)-targets-$(RELEASE_VERSION)-open-$(RELEASE_DATE).zip
endif

# Some paths
REDTEA_RELEASE_APP_TARGETS=$(RELEASE_INSTALL_PATH)/app_targets
REDTEA_RELEASE_APP_SHELLS=$(RELEASE_INSTALL_PATH)/app_shells

# Get github branch and git hash
RELEASE_GIT_PATH=../.git/
GIT_PATH=$(RELEASE_GIT_PATH)
GIT_HEAD_FILE=$(GIT_PATH)/HEAD
ifeq ($(GIT_HEAD_FILE), $(wildcard $(GIT_HEAD_FILE)))
RELEASE_GIT_BRANCH_INFO=$(shell echo $(shell cat $(GIT_HEAD_FILE) | awk '{ print $$2}'))
RELEASE_GIT_BRANCH=$(shell echo $(RELEASE_GIT_BRANCH_INFO) | cut -f 3 -d "/")
RELEASE_GIT_HASH=$(shell echo $(GIT_PATH)/$(RELEASE_GIT_BRANCH_INFO) | xargs cat)
else
RELEASE_GIT_BRANCH=xxxxxxxx
RELEASE_GIT_HASH=xxxxxxxx
endif

# Auto generate README.txt
define CREATE_RELEASE_README
	echo "" >$(1);\
    echo "/* Auto-generated readme for released targets ! */" >$(1);\
    echo "" >>$(1);\
	echo "Release information" >>$(1);\
    echo "Author            : ${RELEASE_AUTHOR}" >>$(1);\
    echo "Time              : ${RELEASE_TIME}" >>$(1);\
	echo "Version           : ${RELEASE_VERSION}" >>$(1);\
	echo "Target            : ${RELEASE_TARGET}" >>$(1);\
	echo "Environment       : ${RELEASE_ENV_TYPE}" >>$(1);\
	echo "Branch            : ${RELEASE_GIT_BRANCH}" >>$(1);\
	echo "Git Hash          : ${RELEASE_GIT_HASH}" >>$(1);\
	echo "STANDARD          : $(RELEASE_STANDARD_MODULE)" >>$(1);\
	echo "" >>$(1);\
	echo "Description of directories" >>$(1);\
	echo "app_shells        : Some shell scripts for autoly run our own appliacations" >>$(1);\
	echo "app_targets       : Some app targets, which can be pushed into module directly or uploaded to OTA server" >>$(1);\
	echo "ubi_flash_targets : A flash ubi image, which can be downloaded into terminal by flash tool(e.g. QFlash tool) only" >>$(1);\
	echo "ubi_ota_targets   : A OTA ubi image, which can be uploaded to OTA server; It's name looks like agent/monitor/libcomm.so which accord with OTA file name requirement " >>$(1);\
	echo "" >>$(1);\
	echo "Manual installation tips" >>$(1);\
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
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*agent* $(REDTEA_RELEASE_APP_TARGETS)
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/*monitor* $(REDTEA_RELEASE_APP_TARGETS)
	-$(Q)cp -rf $(SYSAPP_INSTALL_PATH)/test_lpa $(REDTEA_RELEASE_APP_TARGETS)
	-$(Q)cp -rf $(SDK_PATH)/lib/*-libcomm.so* $(REDTEA_RELEASE_APP_TARGETS)
	-$(Q)cp -rf $(REDTEA_OEMAPP_SKB_SO) $(REDTEA_RELEASE_APP_TARGETS)
	-$(Q)cp -rf $(REDTEA_SHELL_APP) $(REDTEA_RELEASE_APP_SHELLS)
	-$(Q)cp -rf $(REDTEA_SHELL_KEEP) $(REDTEA_RELEASE_APP_SHELLS)
	-$(Q)cp -rf $(REDTEA_SHELL_ADB_PUSH) $(RELEASE_INSTALL_PATH)
	-$(Q)cp -rf $(REDTEA_CHANGE_LOG) $(RELEASE_INSTALL_PATH)
endef

# Tar all release targets
define TAR_RELEASE_TARGETS
	cd $(1)/../; rm -rf $(1)/*.zip; zip -q -r $(2) ./$(shell basename $(1)); mv $(2) $(1)
endef

release:
	@test -e $(REDTEA_RELEASE_APP_TARGETS) || mkdir -p $(REDTEA_RELEASE_APP_TARGETS)
	@test -e $(REDTEA_RELEASE_APP_SHELLS) || mkdir -p $(REDTEA_RELEASE_APP_SHELLS)
	$(Q)$(call COPY_RELEASE_TARGETS)
	$(Q)test -e $(RELEASE_INSTALL_PATH)/$(REDTEA_RELEASE_README) || $(call CREATE_RELEASE_README,$(RELEASE_INSTALL_PATH)/$(REDTEA_RELEASE_README))
	$(Q)$(call TAR_RELEASE_TARGETS,$(RELEASE_INSTALL_PATH),$(RELEASE_TAR_FILE))

release-clean:
	rm -rf $(RELEASE_INSTALL_PATH) $(RELEASE_BUILD_DIR)

.PHONY: release release-clean

endif