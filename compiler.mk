CCVERSIONINFO := $(shell $(CC) --version | tr A-Z a-z)
CXXVERSIONINFO := $(shell $(CXX) --version | tr A-Z a-z)

# TODO: Replace gcc check with test program like a unix configure script as currently this fails for things like cc on ubuntu and msys because they don't have gcc in the --version output
ifneq (,$(findstring gcc,$(CCVERSIONINFO)))
GCCTARGET := $(shell $(CC) -print-multiarch)
ifeq ($(GCCTARGET),)
GCCTARGET := $(shell $(CC) -dumpmachine)
endif
# TODO: download config.sub instead of hardcoding it in the repo
TARGET := $(shell sh ./config.sub $(GCCTARGET))

CSTD := -std=c99
CWARNINGS := -Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
endif

ifneq (,$(findstring clang,$(CCVERSIONINFO)))
CLANGTARGET := $(shell $(CC) -dumpmachine)
TARGET := $(shell ./config.sub $(CLANGTARGET))

CSTD := -std=c99
CWARNINGS := -Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
endif

ifneq (,$(findstring gcc,$(CXXVERSIONINFO)))
GCCTARGET := $(shell $(CXX) -print-multiarch)
ifeq ($(GCCTARGET),)
GCCTARGET := $(shell $(CXX) -dumpmachine)
endif
CXXTARGET := $(shell sh ./config.sub $(GCCTARGET))
ifeq ($(TARGET),$(CXXTARGET))
HAVECXX := TRUE
endif

CXXSTD := -std=c++17
CXXWARNINGS := -Wall -Wextra -pedantic
endif

ifneq (,$(findstring clang,$(CCVERSIONINFO)))
CLANGTARGET := $(shell $(CC) -dumpmachine)
CXXTARGET := $(shell ./config.sub $(CLANGTARGET)))
ifeq ($(TARGET),$(CXXTARGET))
HAVECXX := TRUE
endif

CXXSTD := -std=c++17
CXXWARNINGS := -Wall -Wextra -pedantic
endif

CFLAGS :=
CXXFLAGS :=

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
ifndef TARGET
ifdef CC
$(error Compiler "$(CC)" is not supported)
else
$(error CC variable not set)
endif
endif
endif

ifdef NEEDCXX
ifndef HAVECXX
ifdef CXX
$(error Compiler "$(CXX)" is not supported)
else
$(error CXX variable not set)
endif
endif
endif

ifneq (,$(findstring mingw,$(CCVERSIONINFO)))
ISWINDOWS := TRUE
endif
ifeq ($(OS),Windows_NT)
ISWINDOWS := TRUE
endif

ifneq (,$(findstring cosmo,$(CCVERSIONINFO)))
ISCOSMO := TRUE
endif
