
# Config your target here !
TARGET                  = comm
LIB_SO_NAME             = lib$(TARGET).so
LIB_A_NAME              = lib$(TARGET).a

# Config all static library target
ALL_TMP_TARGETS         = $(foreach d,$(LIBRARY_ALL_TARGETS),$(addprefix $(O)/$(d)/lib,$(d)))
ALL_TARGETS             = $(foreach d,$(ALL_TMP_TARGETS),$(addsuffix .a,$(d)))

# Config your sysapp version: (VERSION_MAJOR).(VERSION_MINOR).(VERSION_REVSION).(VERSION_MODIFY)
COMM_LOCAL_VERSION      = 1.0.0

# Config your final release target name
RELEASE_SHARED_TARGET   = $(LIB_SO_NAME).$(COMM_LOCAL_VERSION)
RELEASE_STATIC_TARGET   = $(LIB_A_NAME).$(COMM_LOCAL_VERSION)

############################################################################################
# Never modify form this line
############################################################################################
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/all_library.mk
############################################################################################
# Never modify end this line
############################################################################################