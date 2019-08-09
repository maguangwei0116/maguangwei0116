

PRE_KCONFIG_TARGET+=.sysapp-pre-kconfig

# Do not use kconfig variable
.sysapp-pre-kconfig:
	$(TOPDIR)/support/scripts/wrapper.sh BR2_CONFIG_SYSAPP_ $(WORKSPACE)/sysapp $(WRAPPER_DIR)/sysapp_wrapper.in 
