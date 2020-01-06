
REDTEA_SUPPORT_SCRIPTS_PATH=$(PWD)/support/scripts
MAKE_BACKUP_PROFILE_ARRAY_TOOL=$(REDTEA_SUPPORT_SCRIPTS_PATH)/make_array.py
CUR_BACKUP_PROFILE=../doc/backup_profile/*.der
FINAL_BACKUP_PROFILE=../sysapp/monitor/core/backup/inc/backup_profile.h

backup_profile: 
	$(Q)$(MAKE_BACKUP_PROFILE_ARRAY_TOOL) $(CUR_BACKUP_PROFILE) $(FINAL_BACKUP_PROFILE)
	
backup_profile-clean: 
	$(Q)rm -rf $(FINAL_BACKUP_PROFILE)

.PHONY: backup_profile backup_profile-clean

