# CC Configuation
CCCOMPILERTARGET := $(shell $(CC) -print-multiarch 2>/dev/null)
ifeq ($(COMPILERTARGET),)
CCCOMPILERTARGET := $(shell $(CC) -dumpmachine 2>/dev/null)
endif
# TODO: download config.sub instead of hardcoding it in the repo
TARGET := $(shell sh build/config.sub $(CCCOMPILERTARGET) 2>/dev/null)

ifdef NEEDCC
ifeq (,$(findstring cosmo,$(TARGET)))
ifeq (,$(findstring gnu,$(TARGET)))
ifeq (,$(findstring mingw,$(TARGET)))
ifdef CC
ifneq (,$(TARGET))
$(error Compiler $(CC) for $(TARGET) is not supported, only Clang and GCC based compilers are supported)
else
$(error Compiler $(CC) is not supported, only Clang and GCC based compilers are supported)
endif # !TARGET
else # !CC
$(error CC variable not set)
endif # CC/!CC
endif # !mingw
endif # !gnu
endif # !cosmo
endif # !clean


# CXX Configuration
CXXCOMPILERTARGET := $(shell $(CXX) -print-multiarch 2>/dev/null)
ifeq ($(COMPILERTARGET),)
CXXCOMPILERTARGET := $(shell $(CXX) -dumpmachine 2>/dev/null)
endif
# TODO: download config.sub instead of hardcoding it in the repo
CXXTARGET := $(shell sh build//config.sub $(CXXCOMPILERTARGET))

ifdef NEEDCXX

ifeq (,$(findstring cosmo,$(TARGET)))
ifeq (,$(findstring gnu,$(CXXTARGET)))
ifeq (,$(findstring mingw,$(CXXTARGET)))
ifdef CXX
ifneq (,$(CXXTARGET))
$(error Compiler $(CXX)" for $(CXXTARGET) is not supported, only Clang and GCC based compilers are supported)
else
$(error Compiler $(CXX) is not supported, only Clang and GCC based compilers are supported)
endif # !TARGET

else # !CXX
$(error CXX variable not set)
endif # CXX/!CXX
endif # !mingw
endif # !gnu
endif # !cosmo

# TODO: Find a way to detect CXX defaulting to g++ vs being set to g++
# Then move this out of the NEEDCXX check so it can report the bad value even if not required
ifneq ($(TARGET),$(CXXTARGET))
$(error Compiler $(CXX) for $(CXXTARGET) does not have the same target as $(CC) for $(TARGET))
endif # TARGET != CXXTARGET

endif # NEEDCXX


# Target environment detection
ifneq (,$(findstring mingw,$(TARGET)))
ISWINDOWS := TRUE
endif
ifeq ($(OS),Windows_NT)
ISWINDOWS := TRUE
endif

ifneq (,$(findstring cosmo,$(TARGET)))
ISCOSMO := TRUE
endif


CSTD := -std=c17
CWARNINGS := -Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition

CXXSTD := -std=c++17
CXXWARNINGS := -Wall -Wextra -pedantic

CFLAGS :=
CXXFLAGS :=


ifeq (,$(findstring clean,$(MAKECMDGOALS)))
$(info Building for $(TARGET))
endif
