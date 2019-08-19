ifeq ($(BR2_PACKAGE_SDK_BUILD),y)

TARGETS += sdk

SDK_PATH=$(call qstrip,$(BR2_SDK_INSTALL_PATH))
SDK_BUILD_DIR=$(BUILD_DIR)/sdk

.PHONY: sdk sdk-clean sdk-upload

sdk:
	@test -e $(SDK_PATH)/lib     || mkdir -p $(SDK_PATH)/lib
	@test -e $(SDK_PATH)/include || mkdir -p $(SDK_PATH)/include
#	@test -e $(SDK_PATH)/bin     || mkdir -p $(SDK_PATH)/bin
#	@test -e $(SDK_PATH)/doc     || mkdir -p $(SDK_PATH)/doc
	
sdk-clean:
	rm -rf $(SDK_PATH) $(SDK_TEMP_DIR)
	
endif
