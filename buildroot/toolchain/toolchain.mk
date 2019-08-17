

.PHONY: toolchain toolchain-clean

TOOLCHAIN_PATH=$(call qstrip,$(BR2_TOOLCHAIN_INSTALL_PATH))
TARGET_CROSS=$(TOOLCHAIN_PATH)/$(call qstrip,$(BR2_TOOLCHAIN_PREFIX))
#$(warning BR2_TOOLCHAIN_SYSROOT=$(BR2_TOOLCHAIN_SYSROOT))
TARGET_SYSROOT=$(call qstrip,$(BR2_TOOLCHAIN_SYSROOT))
TARGET_SYSINC="$(call qstrip,$(BR2_TOOLCHAIN_SYSINC))"
TARGET_LIB_PATH=$(TOOLCHAIN_PATH)/$(dir $(call qstrip,$(BR2_TOOLCHAIN_PREFIX)))/../lib
NDK_CFLAGS="-DTEXT_BASE=0x03000000 -DPREFER_SIZE_OVER_SPEED -DPLATFORM_FLAVOR=PLATFORM_FLAVOR_ID_360S -DTRACE_LEVEL= \
				          -nostdinc -isystem $(shell $(TARGET_CROSS)gcc -print-file-name=include 2> /dev/null)"

toolchain:
	@echo "toolchain building $(TOOLCHAIN_PATH)"
	test -e $(TOOLCHAIN_PATH)/ndk || \
	(wget --output-document=$(DL_DIR)/toolchain-cross.bin $(BR2_TOOLCHAIN_URL) && \
	mkdir -p $(TOOLCHAIN_PATH) && echo "Install toolchain ..." && \
	$(TAR) $(TAR_STRIP_COMPONENTS)=0 -C $(TOOLCHAIN_PATH) $(TAR_OPTIONS) $(DL_DIR)/toolchain-cross.bin)
	rm -rf $(DL_DIR)/toolchain-cross.bin

toolchain-clean:
	@echo "toolchain-clean"
	rm -rf $(TOOLCHAIN_PATH)

