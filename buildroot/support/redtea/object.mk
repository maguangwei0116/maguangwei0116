
COBJS			= $(patsubst %.c, $(O)/%.o, $(SRC-y))
OBJS			= $(patsubst %.s, $(O)/%.o, $(COBJS))
DEPS			= $(foreach dep,$(OBJS),$(dir $(dep)).$(notdir $(dep)).d)

$(O)/%.o : %.c
	@[ -d $(dir $@) ] || $(MKDIR) -p $(dir $@)
#	$($(quiet)do_cc) -c $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@$($(quiet)do_cc) -E $(CFLAGS) -o "$(@:%.o=%.i)" "$<"
	$($(quiet)do_cc) -c $(CFLAGS) -fPIC -MD -MF "$(dir $@).$(notdir $@).d" -o"$@" "$<"

$(O)/%.o : %.s
	@[ -d $(dir $@) ] || $(MKDIR) -p $(dir $@)
#	$($(quiet)do_as) -c $(ASFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	$($(quiet)do_as) -c $(ASFLAGS) -MD -MF "$(dir $@).$(notdir $@).d" -o"$@" "$<"