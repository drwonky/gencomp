MACH = $(shell uname -m)

ifeq ($(MACH), i686)

ARCH = linux_x86

CC = gcc 
CC_SO = gcc -shared
CC_PIC = gcc -fPIC
CC_MT = gcc -D_REENTRANT

GCC_BC = /home/pedward/gcc-2.7.2/xgcc -B/home/pedward/gcc-2.7.2/ -fbounds-checking
GCC = gcc
GCC_SO = gcc -shared

INSTALL = /usr/bin/install

else

ARCH = solaris_sparc
SPRO = /opt/SUNWspro/bin/cc 
CC = $(SPRO)
CC_SO = $(SPRO) -G
CC_PIC = $(SPRO) -KPIC
CC_MT = $(SPRO) -mt
CC_SO_MT = $(SPRO) -G -mt

GCC = /usr/local/bin/gcc
GCC_SO = /usr/local/bin/gcc -shared

INSTALL = /usr/local/bin/install

endif

TOP_DIR = /opt/webcom/dev

# because other trees may want our build tools
GLOBAL_BIN_DIR = $(TOP_DIR)/c/bin/$(ARCH)
GLOBAL_TOOLS_DIR = $(GLOBAL_BIN_DIR)/tools
GLOBAL_INCLUDE = $(TOP_DIR)/c/include
GLOBAL_LIB = $(TOP_DIR)/c/lib/$(ARCH)


INSTALL_A = $(INSTALL) -m 0664 -g prog
INSTALL_SO = $(INSTALL) -m 0664 -g prog
INSTALL_BIN = $(INSTALL) -m 0775 -g prog
