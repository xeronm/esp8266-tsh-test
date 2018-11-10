## Customizable Section: adapt those variables to suit your program.
##==========================================================================

# The executable file name.
# If not specified, current directory name or `a.out' will be used.
APP   = tsh-test

APP_SOURCE = ./../tsh

# The directories in which header files reside.
INCLUDES_APP = ./include ./include/arch ./include/arch/default

# The directories in which source files reside.
# If not specified, only the current directory will be serached.
SRCDIRS_APP   = ./arch/default ./crypto ./misc ./proto ./core ./system ./service ./service/device

INCLUDES_EXTRA = ./../googletest/googletest/include

LIBDIR_EXTRA = 

OBJ_EXTRA = ./../googletest/googletest/make/gtest_main.a

# DEFINES
#    DISABLE_SERVICES -
#    DISABLE_CORE -
#    DISABLE_SYSTEM -
#    LOGGING_DEBUG -
#    LOGGING_DEBUG_MODE -
DEFINES = LOGGING_DEBUG ASSERT_DEBUG LOGGING_SEVERITY=LOG_ERROR

LIBS = pthread

INCLUDES = $(addprefix $(APP_SOURCE)/,$(INCLUDES_APP)) $(INCLUDES_APP)
SRCDIRS = $(addprefix $(APP_SOURCE)/,$(SRCDIRS_APP)) $(SRCDIRS_APP)

CFLAGS = $(addprefix -I,$(INCLUDES)) \
	$(addprefix -I,$(INCLUDES_EXTRA)) \
	$(addprefix -D,$(DEFINES)) \
	-fno-strict-aliasing \
	-fno-omit-frame-pointer \
	-fexceptions \
	-g2 -gdwarf-2 \
	-std=gnu11

# The pre-processor options used by the cpp (man cpp for more).
CPPFLAGS  = $(addprefix -I,$(INCLUDES)) \
	$(addprefix -I,$(INCLUDES_EXTRA)) \
	$(addprefix -D,$(DEFINES)) \
	-fno-strict-aliasing \
	-fno-omit-frame-pointer \
	-fthreadsafe-statics \
	-fexceptions \
	-frtti \
	-g2 -gdwarf-2 \
	-std=gnu++11

LDLIBS =  $(addprefix -l,$(LIBS))
LDFLAGS = $(addprefix -L,$(LIBDIR_EXTRA))


## Implicit Section: change the following only when necessary.
##==========================================================================
BUILD_DIR = .build/$(APP)/
BINDIR = bin/

# The source file types (headers excluded).
# .c indicates C source files, and others C++ ones.
SRCEXTS = .c .cc .cpp .c++ .cxx .cp

# The header file types.
HDREXTS = .h .hh .hpp .h++ .hxx .hp

# C/C++ Compilers
CC = gcc
CXX = g++
OBJCOPY = objcopy

# Used shell command
RM     = rm -f
MKDIR  = mkdir -p
CP     = cp -v
MV     = mv -v
PYTHON = python

## Stable Section: usually no need to be changed. But you can add more.
##==========================================================================
SHELL   = /bin/sh
EMPTY   =
SPACE   = $(EMPTY) $(EMPTY)

SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
HEADERS = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(HDREXTS))))
SRC_CXX = $(filter-out %.c,$(SOURCES))
OBJS    = $(addprefix $(BUILD_DIR),$(addsuffix .o, $(basename $(SOURCES))))
DEPS    = $(OBJS:.o=.d)

## Define some useful variables.
DEP_OPT = $(shell if `$(CC) --version | grep "GCC" >/dev/null`; then \
                  echo "-MM -MP"; else echo "-M"; fi )

DEPEND      = $(CC)  $(DEP_OPT)  $(CFLAGS) $(CPPFLAGS)
DEPEND.d    = $(subst -g ,,$(DEPEND))
COMPILE.c   = $(CC)  $(CFLAGS)   -c
COMPILE.cxx = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c
LINK.c      = $(CC)  $(CFLAGS)   $(LDFLAGS) $(LDLIBS) 
LINK.cxx    = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

.PHONY: all objs clean show buildpath project

# Delete the default suffixes
.SUFFIXES:

all: buildpath objs project

buildpath:
	$(MKDIR) $(BUILD_DIR)
	$(MKDIR) $(addprefix $(BUILD_DIR),$(SRCDIRS))

# Rules for generating object files (.o).
#----------------------------------------
objs:$(OBJS)

$(BUILD_DIR)%.o:%.c
	$(COMPILE.c) $< -o $@

$(BUILD_DIR)%.o:%.cc
	$(COMPILE.cxx) $< -o $@

$(BUILD_DIR)%.o:%.cpp
	$(COMPILE.cxx) $< -o $@

$(BUILD_DIR)%.o:%.c++
	$(COMPILE.cxx) $< -o $@

$(BUILD_DIR)%.o:%.cp
	$(COMPILE.cxx) $< -o $@

$(BUILD_DIR)%.o:%.cxx
	$(COMPILE.cxx) $< -o $@

# Rules for generating the executable.
#-------------------------------------
project: $(BINDIR)$(APP)

$(BINDIR)$(APP): %:$(OBJS)
ifeq ($(SRC_CXX),)              # C program
	$(LINK.c)   $(OBJS) $(OBJ_EXTRA) -o $@
else                            # C++ program
	$(LINK.cxx) $(OBJS) $(OBJ_EXTRA) -o $@
endif

clean:
	$(RM) $(OBJS) $(APPS)

cleanall: clean
	$(RM) -r $(BUILD_DIR) $(BINDIR)

# Show variables (for debug use only.)
show:
	@echo '*******************************************************'
	@echo 'TEST_APP    :' $(TEST_APP)
	@echo 'SRCDIRS     :' $(SRCDIRS)
	@echo 'HEADERS     :' $(HEADERS)
	@echo 'SOURCES     :' $(SOURCES)
	@echo 'SRC_CXX     :' $(SRC_CXX)
	@echo 'OBJS        :' $(OBJS)
	@echo 'DEPS        :' $(DEPS)
	@echo 'DEPEND      :' $(DEPEND)
	@echo 'COMPILE.c   :' $(COMPILE.c)
	@echo 'COMPILE.cxx :' $(COMPILE.cxx)
	@echo 'link.c      :' $(LINK.c)
	@echo 'link.cxx    :' $(LINK.cxx)
	@echo '*******************************************************'
