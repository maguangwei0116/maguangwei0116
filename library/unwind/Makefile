
# Config your target here !
TARGET 				= $(shell pwd | sed 's/^\(.*\)[/]//')
LIB_A_NAME  		= lib$(TARGET).a
LIBUNWIND   		= libunwind-1.3.1
HOST				= arm-linux
CC					= $(CROSS_COMPILE)gcc
AR					= $(CROSS_COMPILE)ar
SRC-y 				= backtrace_libunwind.c
OBJ-y 				= $(O)/backtrace_libunwind.o
SRC_MAKEFILE 		= src/Makefile
LIBUNWIND_LIB		= $(SDK_INSTALL_PATH)/lib
LIBUNWIND_INCLUDE	= $(SDK_INSTALL_PATH)/include
LIBUNWIND_OBJ-y		:= $(shell find $(LIBUNWIND) -name *.o)
LIBUNWIND_ALLOBJ-y	= $(O)/$(LIBUNWIND)/*.o
CFLAGS				+= -g -rdynamic -mapcs-frame -funwind-tables -fasynchronous-unwind-tables -I$(LIBUNWIND_INCLUDE) 

all: info final_install_libunwind

info: 
#	@echo CC=$(CC)
#	@echo SDK_INSTALL_PATH=$(SDK_INSTALL_PATH)
#	@echo LIBUNWIND_OBJ-y=$(LIBUNWIND_OBJ-y)
	
configure:
	@cd $(LIBUNWIND); [ -f $(SRC_MAKEFILE) ] || ./configure CC=$(CC) --host=$(HOST)

make_libunwind: configure
	@make -C $(LIBUNWIND)

install_libunwind: make_libunwind
	@[ -d $(LIBUNWIND_LIB) ] || mkdir -p $(LIBUNWIND_LIB)
	@[ -d $(LIBUNWIND_INCLUDE) ] || mkdir -p $(LIBUNWIND_INCLUDE)
	@cp -rf $(LIBUNWIND)/include/*unwind*.h $(LIBUNWIND_INCLUDE)
	@[ -d $(O)/$(LIBUNWIND) ] || mkdir -p $(O)/$(LIBUNWIND)
#	@cp -rf $(LIBUNWIND_OBJ-y) $(O)/$(LIBUNWIND)/
	@cp -rf $(LIBUNWIND)/src/.libs/$(LIB_A_NAME) $(O)/$(LIBUNWIND)/
	@cd $(O)/$(LIBUNWIND)/; $(AR) -x $(LIB_A_NAME)

$(OBJ-y): install_libunwind

backtrace_libunwind: $(OBJ-y)

$(O)/%.o : %.c
	@[ -d $(dir $@) ] || $(MKDIR) -p $(dir $@)
	@$(CC) -E $(CFLAGS) -o "$(@:%.o=%.i)" "$<"
	@$(CC) -c $(CFLAGS) -fPIC -MD -MF "$(dir $@).$(notdir $@).d" -o"$@" "$<"	

$(O)/$(LIB_A_NAME): $(OBJ-y)
	@$(AR) cru "$@" $(OBJ-y) $(LIBUNWIND_ALLOBJ-y)
	@rm -rf $(LIBUNWIND_ALLOBJ-y)
	
final_install_libunwind: backtrace_libunwind $(O)/$(LIB_A_NAME)
	@cp -rf $(O)/$(LIB_A_NAME) $(LIBUNWIND_LIB)

clean:
	@make -C $(LIBUNWIND) clean
	@rm -rf $(O)

.PHONY: all clean info configure make_libunwind install_libunwind backtrace_libunwind final_install_libunwind