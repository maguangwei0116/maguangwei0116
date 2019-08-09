ifeq ($(BR2_PACKAGE_SDK_BUILD),y)

TARGETS += sdk

SDK_PATH=$(call qstrip,$(BR2_SDK_INSTALL_PATH))
SDK_BUILD_DIR=$(BUILD_DIR)/sdk

USER=`whoami`
HOST_IP=10.1.40.23

define SVN_VERSION_NO
$(patsubst sdk-download-svn%,%,$@)
endef

SDK_LASTEST_FILE=sdk-lastest.tar.bz2
SDK_DOWNLOAD_URL="http://$(HOST_IP)/pub/sdk/$(PRODUCT_NAME)/$(SDK_LASTEST_FILE)"
SDK_VERSION_FILE=sdk-svn$(SVN_VERSION_NO)
SDK_VERSION_DOWNLOAD_URL="http://$(HOST_IP)/pub/sdk/$(PRODUCT_NAME)/$(SDK_VERSION_FILE)"
SDK_UPLOAD_DEST="$(USER)@$(HOST_IP):/opt/www/pub/sdk/$(PRODUCT_NAME)"

PACKED_DATE = `date +'%Y%m%d'`
TEE_IMPL_VERSION ?= $(shell svn info 2>/dev/null | grep '^Revision' | awk '{print $$NF}')
SDK_VERSION=$(call qstrip,$(BR2_IMAGE_VERSION))
SDK_PKG_NAME=sdk-$(SDK_PRODUCT_NAME)-$(SDK_PLATFORM_TYPE)-$(SDK_FIRMWARE_TYPE)-$(PACKED_DATE)-svn$(TEE_IMPL_VERSION)-v$(SDK_VERSION).tar.bz2

.PHONY: sdk sdk-clean sdk-upload

sdk:
	@test -e $(SDK_PATH)/lib     || mkdir -p $(SDK_PATH)/lib
	@test -e $(SDK_PATH)/include || mkdir -p $(SDK_PATH)/include
#	@test -e $(SDK_PATH)/bin     || mkdir -p $(SDK_PATH)/bin
#	@test -e $(SDK_PATH)/doc     || mkdir -p $(SDK_PATH)/doc
	
sdk-clean:
	rm -rf $(SDK_PATH) $(SDK_TEMP_DIR)

sdk-download-svn%: sdk
	@rm -rf $(SDK_BUILD_DIR)/$(SDK_VERSION_FILE)
	@mkdir -p $(SDK_BUILD_DIR)
	echo "" && echo "Downloading sdk (svn=$(SVN_VERSION_NO)) ..." && echo ""
	wget --output-document=$(SDK_BUILD_DIR)/$(SDK_VERSION_FILE) $(SDK_VERSION_DOWNLOAD_URL)
	rm -rf $(SDK_PATH)
	$(TAR) -C $(dir $(SDK_PATH)) $(TAR_OPTIONS) $(SDK_BUILD_DIR)/$(SDK_VERSION_FILE)
	echo "" && echo "Download sdk (svn=$(SVN_VERSION_NO)) ok !!!" && echo ""	
	
sdk-download: sdk
	@rm -rf $(SDK_BUILD_DIR)/$(SDK_LASTEST_FILE)
	@mkdir -p $(SDK_BUILD_DIR)
	wget --output-document=$(SDK_BUILD_DIR)/$(SDK_LASTEST_FILE) $(SDK_DOWNLOAD_URL)
	rm -rf $(SDK_PATH)
	$(TAR) -C $(dir $(SDK_PATH)) $(TAR_OPTIONS) $(SDK_BUILD_DIR)/$(SDK_LASTEST_FILE)
	echo "" && echo "Download lastest sdk ok !!!" && echo ""
	
sdk-upload: 
	@rm -rf $(SDK_BUILD_DIR)/upload
	@mkdir -p $(SDK_BUILD_DIR)/upload
	cd $(dir $(SDK_PATH)); $(TAR) jcf $(SDK_BUILD_DIR)/upload/$(SDK_PKG_NAME) sdk
	cp $(SDK_BUILD_DIR)/upload/$(SDK_PKG_NAME) $(SDK_BUILD_DIR)/upload/$(SDK_LASTEST_FILE)
	@echo "" && echo "Releasing lastest sdk to $(SDK_UPLOAD_DEST) ... Pls input your passwd ..." && echo "";\
	scp -r $(SDK_BUILD_DIR)/upload/. $(SDK_UPLOAD_DEST)	&& echo "" && echo "Released lastest sdk ok !!!" && echo ""
	
endif
