
############################################################################################
# Never modify form this line
############################################################################################

all: $(TARGET)

LIB_ANDROID_AGENT_SO_NAME           = libagent.so

LIBSSL_A                            = $(SYSROOT)/usr/lib/aarch64-linux-android/libssl.a
LIBCRYPTO_A                         = $(SYSROOT)/usr/lib/aarch64-linux-android/libcrypto.a

OPENSSL_SSL_OBJS_PATH               = $(O)/openssl/ssl
OPENSSL_CRYPTO_OBJS_PATH            = $(O)/openssl/crypto
SSL_OBJS_LIST                       := $(shell ar -t $(LIBSSL_A))
CRYPTO_OBJS_LIST                    := $(shell ar -t $(LIBCRYPTO_A))
SSL_OBJS                            = $(patsubst %, $(OPENSSL_SSL_OBJS_PATH)/%, $(SSL_OBJS_LIST))
CRYPTO_OBJS                         = $(patsubst %, $(OPENSSL_CRYPTO_OBJS_PATH)/%, $(CRYPTO_OBJS_LIST))

ANDROID_AGENT_ORG_OBJS              = $(patsubst %/main.o, , $(OBJS))  # delete main.o
ANDROID_LIBRARIES_OBJS              = $(shell find $(O)/../../library -name *.o)
ANDROID_AGENT_OBJS                  = $(ANDROID_LIBRARIES_OBJS) $(ANDROID_AGENT_ORG_OBJS) 
ANDROID_AGENT_WITH_OPENSSL_OBJS     = $(ANDROID_LIBRARIES_OBJS) $(ANDROID_AGENT_ORG_OBJS) $(SSL_OBJS) $(CRYPTO_OBJS)

gen_openssl_objs:
	@test -e $(OPENSSL_SSL_OBJS_PATH) || mkdir -p $(OPENSSL_SSL_OBJS_PATH)
	@test -e $(OPENSSL_CRYPTO_OBJS_PATH) || mkdir -p $(OPENSSL_CRYPTO_OBJS_PATH)
	cd $(OPENSSL_SSL_OBJS_PATH); ar -x $(LIBSSL_A)
	cd $(OPENSSL_CRYPTO_OBJS_PATH); ar -x $(LIBCRYPTO_A)
#	@echo SSL_OBJS=$(SSL_OBJS)
#	@echo CRYPTO_OBJS=$(CRYPTO_OBJS)

android_info:
	@echo "ANDROID_AGENT_WITH_OPENSSL_OBJS=$(ANDROID_AGENT_WITH_OPENSSL_OBJS)"

# Include sub comm makefiles
-include $(REDTEA_SUPPORT_REDTEA_PATH)/tool.mk

$(TARGET): $(O)/$(LIB_ANDROID_AGENT_SO_NAME)

$(O)/$(LIB_ANDROID_AGENT_SO_NAME): gen_openssl_objs $(ANDROID_AGENT_OBJS)
	$($(quiet)do_link) -shared -Wl,-soname=$(LIB_ANDROID_AGENT_SO_NAME) $(ANDROID_AGENT_WITH_OPENSSL_OBJS)  -o"$@"
	$(STRIP_ALL) "$@"
	-$(Q)$(CP) -rf $@ $(SDK_INSTALL_PATH)/lib
	@$(ECHO) ""
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) "|"
	@$(ECHO) "|   Finished building target: $(LIB_ANDROID_AGENT_SO_NAME)"
	@$(ECHO) "|"
	@$(ECHO) "+---------------------------------------------------"
	@$(ECHO) ""

.PHONY: all android_info gen_openssl_objs $(TARGET)

# Include the dependency files, should be the last of the makefile
-include $(DEPS)

############################################################################################
# Never modify end this line
############################################################################################