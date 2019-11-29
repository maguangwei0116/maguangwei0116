
############################################################################################
# Never modify form this line
############################################################################################

TEST_TARGET				= test_lpa
TEST_TARGET_FILE_NAME 	= $(TEST_TARGET)
TEST_ELF_FILE_NAME 		= $(TEST_TARGET).elf
TEST_MAP_FILE_NAME 		= $(TEST_TARGET).map
TEST_DMP_FILE_NAME 		= $(TEST_TARGET).dmp
TEST_BIN_FILE_NAME 		= $(TEST_TARGET).bin

# test lpa tool
TEST_LPA_C              = ./main/src/test_lpa.c
TEST_LPA_O			    = $(patsubst %.c, $(O)/%.o, $(TEST_LPA_C))
TEST_LPA_OBJS           = $(patsubst %/main.o, , $(OBJS))  # delete main.o
TEST_LPA_LIB_OBJS       = $(shell find $(O)/../../library -name *.o)
TEST_LPA_TOTAL_OBJS     += $(TEST_LPA_O) $(TEST_LPA_OBJS) $(TEST_LPA_LIB_OBJS)
TEST_LPA_LDFLAGS        = $(patsubst -lcomm, , $(LDFLAGS))  # delete -lcomm

#$(warning TEST_LPA_TOTAL_OBJS=$(TEST_LPA_TOTAL_OBJS))

all: $(TEST_TARGET)

$(TEST_TARGET): $(O)/$(TEST_TARGET_FILE_NAME)

# Every object file depend on conf-file
$(TEST_LPA_O): $(conf-file)

$(O)/$(TEST_TARGET_FILE_NAME): $(TEST_LPA_TOTAL_OBJS)
	$($(quiet)do_link) -o "$@" -Wl,--whole-archive $(TEST_LPA_TOTAL_OBJS) -Wl,--no-whole-archive $(TEST_LPA_LDFLAGS) -Wl,-Map=$(O)/$(MAP_FILE_NAME) 
	$($(quiet)do_objdump) -l -x -d "$@" > $(O)/$(TEST_DMP_FILE_NAME)
	$($(quiet)do_copy) -O binary -S "$@" $(O)/$(TEST_BIN_FILE_NAME)
	@$(CHMOD) +x "$@"
	$(STRIP_ALL) "$@"
	-$(Q)$(CP) -rf $(O)/$(TEST_TARGET_FILE_NAME) $(O)/$(TEST_ELF_FILE_NAME)
	-$(Q)$(CP) -rf $(O)/$(TEST_TARGET_FILE_NAME) $(SYSAPP_INSTALL_PATH)/

.PHONY: all $(TEST_TARGET) FORCE

# Include the dependency files, should be the last of the makefile
-include $(DEPS)

############################################################################################
# Never modify end this line
############################################################################################