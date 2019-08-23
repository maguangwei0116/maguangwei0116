
conf-file 			= $(O)/include/generated/conf.h
BR2_PREFIX			= BR2_
BR2_CFG_PRIFIX		= $(BR2_PREFIX)CFG_

# Add a new CFLAG
CFLAGS				+= -include $(conf-file)

# Auto generate conf.h
define LOCAL_AUTO_GEN_CONF_H
    cnf="`cat $(1) | grep $(BR2_CFG_PRIFIX)`";\
    guard="_`echo $(2) | tr -- -/. ___`_";\
    echo "/* Auto-generated configuration header file, never modify it ! */" >$(1);\
    echo "" >>$(1);\
    echo "#ifndef $${guard}" >>$(1);\
    echo "#define $${guard}" >>$(1);\
    echo "" >>$(1);\
    echo -n "$${cnf}" | sed 's/_nl_ */\n/g' >>$(1);\
    echo "" >>$(1);\
    echo "" >>$(1);\
    echo "#endif" >>$(1);\
    sed -i 's/$(BR2_PREFIX)//' $(1);\
	mv $(1) $(2)
endef

# Auto generate conf.h
define AUTO_GEN_CONF_H
    cnf="`cat $(1) | grep $(BR2_CFG_PRIFIX)`";\
	vars="`echo -n "$${cnf}" | sed 's/_nl_ */\n/g'`";\
    guard="_`echo $(2) | tr -- -/. ___`_";\
	tips="/* Auto-generated configuration header file, never modify it ! */";\
    echo -e -n "$${tips}\n\n#ifndef $${guard}\n#define $${guard}\n\n$${vars}\n\n#endif\n" >$(1);\
    sed -i 's/$(BR2_PREFIX)//' $(1);\
	mv $(1) $(2)
endef

gen-conf-file: $(conf-file)

$(conf-file): 
	$(Q)test -e $(dir $@) || $(MKDIR) -p $(dir $@)
	$(Q)$(CP) -rf $(SYSAPP_KCONFIG_AUTOHEADER) $@.tmp
	$(Q)$(call AUTO_GEN_CONF_H,$@.tmp,$@)