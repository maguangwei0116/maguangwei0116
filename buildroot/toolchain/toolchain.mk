
.PHONY: toolchain toolchain-clean

TOOLCHAIN_PATH=$(call qstrip,$(BR2_TOOLCHAIN_INSTALL_PATH))
TARGET_CROSS=$(TOOLCHAIN_PATH)/$(call qstrip,$(BR2_TOOLCHAIN_PREFIX))
#$(warning BR2_TOOLCHAIN_SYSROOT=$(BR2_TOOLCHAIN_SYSROOT))
TARGET_SYSROOT=$(call qstrip,$(BR2_TOOLCHAIN_SYSROOT))
TARGET_SYSINC="$(call qstrip,$(BR2_TOOLCHAIN_SYSINC))"
TARGET_LIB_PATH=$(TOOLCHAIN_PATH)/$(dir $(call qstrip,$(BR2_TOOLCHAIN_PREFIX)))/../lib
