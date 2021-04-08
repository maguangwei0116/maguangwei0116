
############################################################################################
# Never modify form this line
############################################################################################

TEST_CLIENT_TARGET				= test_client
TEST_CLIENT_TARGET_FILE_NAME 	= $(TEST_CLIENT_TARGET)
TEST_ELF_FILE_NAME 		= $(TEST_CLIENT_TARGET).elf
TEST_MAP_FILE_NAME 		= $(TEST_CLIENT_TARGET).map
TEST_DMP_FILE_NAME 		= $(TEST_CLIENT_TARGET).dmp
TEST_BIN_FILE_NAME 		= $(TEST_CLIENT_TARGET).bin

# test client tool
TEST_CLIENT_C           = ./main/src/test_client.c
TEST_CLIENT_O			= $(patsubst %.c, $(O)/%.o, $(TEST_CLIENT_C))
TEST_CLIENT_OBJS        = $(patsubst %/main.o, , $(OBJS))  # delete main.o
TEST_CLIENT_LIB_OBJS    = $(shell find $(O)/../../library -name *.o)
TEST_CLIENT_TOTAL_OBJS  += $(TEST_CLIENT_O) $(TEST_CLIENT_OBJS) $(TEST_CLIENT_LIB_OBJS)
TEST_CLIENT_LDFLAGS     = $(patsubst -lcomm, , $(LDFLAGS))  # delete -lcomm

#$(warning TEST_CLIENT_TOTAL_OBJS=$(TEST_CLIENT_TOTAL_OBJS))

all: $(TEST_CLIENT_TARGET)

$(TEST_CLIENT_TARGET): $(O)/$(TEST_CLIENT_TARGET_FILE_NAME)

# Every object file depend on conf-file
$(TEST_CLIENT_O): $(conf-file)

$(O)/$(TEST_CLIENT_TARGET_FILE_NAME): $(TEST_CLIENT_TOTAL_OBJS)
	$($(quiet)do_link) -o "$@" -Wl,--whole-archive $(TEST_CLIENT_TOTAL_OBJS) -Wl,--no-whole-archive $(TEST_CLIENT_LDFLAGS) -Wl,-Map=$(O)/$(MAP_FILE_NAME) 
#	$($(quiet)do_objdump) -l -x -d "$@" > $(O)/$(TEST_DMP_FILE_NAME)
#	$($(quiet)do_copy) -O binary -S "$@" $(O)/$(TEST_BIN_FILE_NAME)
	@$(CHMOD) +x "$@"
	$(STRIP_ALL) "$@"
	-$(Q)$(CP) -rf $(O)/$(TEST_CLIENT_TARGET_FILE_NAME) $(O)/$(TEST_ELF_FILE_NAME)
	-$(Q)$(CP) -rf $(O)/$(TEST_CLIENT_TARGET_FILE_NAME) $(SYSAPP_INSTALL_PATH)/

.PHONY: all $(TEST_CLIENT_TARGET) FORCE

# Include the dependency files, should be the last of the makefile
-include $(DEPS)

############################################################################################
# Never modify end this line
############################################################################################