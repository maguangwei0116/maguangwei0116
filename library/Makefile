
# Config your target here !
TARGET 				= comm
LIB_SO_NAME 		= lib$(TARGET).so
LIB_A_NAME  		= lib$(TARGET).a

# Config all static library target
ALL_TMP_TARGETS 	= $(foreach d,$(LIBRARY_ALL_TARGETS),$(addprefix $(O)/$(d)/lib,$(d)))
ALL_TARGETS 		= $(foreach d,$(ALL_TMP_TARGETS),$(addsuffix .a,$(d)))

############################################################################################
# Never modify form this line
############################################################################################
-include $(REDTEA_SUPPORT_SCRIPTS_PATH)/all_library.mk
############################################################################################
# Never modify end this line
############################################################################################