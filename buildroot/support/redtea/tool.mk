
# Config for common tools

CC					= $(CROSS_COMPILE)gcc
CXX					= $(CROSS_COMPILE)g++
AS					= $(CROSS_COMPILE)gcc
AR					= $(CROSS_COMPILE)ar
LINK				= $(CROSS_COMPILE)gcc
RANLIB				= $(CROSS_COMPILE)ranlib
OBJCOPY				= $(CROSS_COMPILE)objcopy
OBJDUMP				= $(CROSS_COMPILE)objdump
STRIP				= $(CROSS_COMPILE)strip

TOUCH 				= touch
MKDIR				= mkdir
ECHO				= echo
DEL					= rm
CHMOD				= chmod
CP					= cp
LN					= ln
TR 					= tr
CD					= cd
MV 					= mv
ifeq ($(MYCAT),)
CAT 				= cat
else
CAT 				= $(MYCAT)
endif
ifeq ($(MYSED),)
SED 				= sed
else
SED 				= $(MYSED)
endif

# Force to config SHELL, or [echo -e] will be a problem !
SHELL				= /bin/bash

# Config for quite tool
quiet_do_cc        	= $(Q)$(ECHO)	"  CC       $<" && $(CC)
quiet_do_cxx       	= $(Q)$(ECHO)	"  CXX      $<" && $(CXX)
quiet_do_as        	= $(Q)$(ECHO)	"  AS       $<" && $(AS)
quiet_do_ar        	= $(Q)$(ECHO)	"  AR       $@" && $(AR)
quiet_do_ranlib    	= $(Q)$(ECHO)	"  RANLIB   $@" && $(RANLIB)
quiet_do_copy      	= $(Q)$(ECHO)	"  OBJCOPY  $@" && $(OBJCOPY)
quiet_do_objdump   	= $(Q)$(ECHO)	"  OBJDUMP  $@" && $(OBJDUMP)
quiet_do_link      	= $(Q)$(ECHO)	"  LINK     $@" && $(LINK)
quiet_do_strip     	= $(Q)$(ECHO)	"  STRIP    $@" && $(STRIP)
quiet_do_mkver     	= $(Q)$(ECHO)	"  MAKEVER  $@" &&
quiet_do_transfer  	= $(Q)$(ECHO)	"  TRANSF   $@" && $(TRANSFER)
quiet_do_compress  	= $(Q)$(ECHO)	"  COMPRES  $@" && $(COMPRESS)

do_cc              	= $(CC)
do_cxx             	= $(CXX)
do_as              	= $(AS)
do_ar              	= $(AR)
do_ranlib          	= $(RANLIB)
do_copy            	= $(OBJCOPY)
do_objdump         	= $(OBJDUMP)
do_link            	= $(LINK)
do_strip           	= $(STRIP)
do_mkver           	=
do_transfer        	= $(TRANSFER)
do_compress        	= $(COMPRESS)

ifeq ($(CFG_STRIP_strip),y)
STRIP_ALL           = $($(quiet)do_strip) --strip-all
else
STRIP_ALL           = $(Q)$(TOUCH)
endif
