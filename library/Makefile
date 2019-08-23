
# Config your target here !
TARGET                     = comm
LIB_SO_NAME                = lib$(TARGET).so
LIB_A_NAME                 = lib$(TARGET).a

# Config all static library target
ALL_TMP_TARGETS            = $(foreach d,$(LIBRARY_ALL_TARGETS),$(addprefix $(O)/$(d)/lib,$(d)))
ALL_TARGETS                = $(foreach d,$(ALL_TMP_TARGETS),$(addsuffix .a,$(d)))

# Config your environment, [staging] or [prod]
ENV_TYPE				   = prod

# Config your sysapp version: (VERSION_MAJOR).(VERSION_MINOR).(VERSION_REVSION).(VERSION_MODIFY)
COMM_LOCAL_VERSION         = 1.0.0

# Config your final release target name
RELEASE_SHARED_TARGET      = $(LIB_SO_NAME).$(COMM_LOCAL_VERSION)
RELEASE_STATIC_TARGET      = $(LIB_A_NAME).$(COMM_LOCAL_VERSION)

# Config your final release target name
LIBRARY_TARGET_NAME        = $(LIBRARY_SYSTEM_NAME)-$(LIBRARY_PRODUCT_NAME)-lib$(TARGET)-$(LIBRARY_SOFTWARE_NAME)
LIBRARY_RELEASE_TARGET 	   = $(LIBRARY_TARGET_NAME)_v$(COMM_LOCAL_VERSION)_$(PLATFORM_TYPE)_$(ENV_TYPE)_$(DEBUG_TYPE)
LIBRARY_SHARED_TARGET_NAME = $(LIBRARY_RELEASE_TARGET).so
LIBRARY_STATIC_TARGET_NAME = $(LIBRARY_RELEASE_TARGET).a

# Config your application target version with git hash and build time
RELEASE_TARGET_VERSION 	   = $(LIBRARY_RELEASE_TARGET)_$(SYSAPP_GIT_HASH)_$(shell date +%Y-%m-%d_%H:%M:%S)

############################################################################################
# Never modify form this line
############################################################################################
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/all_library.mk
############################################################################################
# Never modify end this line
############################################################################################