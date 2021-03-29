
conf-file 			    = $(O)/include/generated/conf.h
BR2_DEFINE              = define
BR2_PREFIX			    = BR2_
BR2_CFG                 = CFG_
BR2_CFG_PRIFIX		    = "$(BR2_DEFINE) $(BR2_PREFIX)$(BR2_CFG)"
BR2_CFG_PRIFIX_SED_OLD  = $(BR2_PREFIX)$(BR2_CFG)
BR2_CFG_PRIFIX_SED_NEW  = $(BR2_CFG)

# Add a new CFLAG
CFLAGS				    += -include $(conf-file)

# Auto generate conf.h
define LOCAL_AUTO_GEN_CONF_H
    cnf="`$(CAT) $(1) | grep $(BR2_CFG_PRIFIX)`";\
    guard="_`$(ECHO) $(2) | $(TR) -- -/. ___`_";\
    $(ECHO) "/* Auto-generated configuration header file, never modify it ! */" >$(1);\
    $(ECHO) "" >>$(1);\
    $(ECHO) "#ifndef $${guard}" >>$(1);\
    $(ECHO) "#define $${guard}" >>$(1);\
    $(ECHO) "" >>$(1);\
    $(ECHO) -n "$${cnf}" | $(SED) 's/_nl_ */\n/g' >>$(1);\
    $(ECHO) "" >>$(1);\
    $(ECHO) "" >>$(1);\
    $(ECHO) "#endif" >>$(1);\
    $(SED) -i 's/$(BR2_PREFIX)//' $(1);\
	$(MV) $(1) $(2)
endef

# Auto generate conf.h
define AUTO_GEN_CONF_H
    cnf="`$(CAT) $(1) | grep $(BR2_CFG_PRIFIX)`";\
	vars="`$(ECHO) -n "$${cnf}" | $(SED) 's/_nl_ */\n/g'`";\
    guard="_`$(ECHO) $(2) | $(TR) -- -/. ___`_";\
	tips="/* Auto-generated configuration header file, never modify it ! */";\
    $(ECHO) -e -n "$${tips}\n\n#ifndef $${guard}\n#define $${guard}\n\n$${vars}\n\n#endif\n" >$(1);\
    $(SED) -i 's/$(BR2_CFG_PRIFIX_SED_OLD)/$(BR2_CFG_PRIFIX_SED_NEW)/' $(1);\
	$(MV) $(1) $(2)
endef

$(conf-file):
	$(Q)test -e $(dir $@) || $(MKDIR) -p $(dir $@)
	$(Q)$(CP) -rf $(KCONFIG_AUTOHEADER) $@.tmp.h
	$(Q)$(call AUTO_GEN_CONF_H,$@.tmp.h,$@)