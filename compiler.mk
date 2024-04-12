CCVERSIONINFO := $(shell $(CC) --version | tr A-Z a-z)

# TODO: Replace gcc check with test program like a unix configure script as currently this fails for things like cc on ubuntu and msys because they don't have gcc in the --version output
ifneq (,$(findstring gcc,$(CCVERSIONINFO)))
GCCTARGET := $(shell $(CC) -print-multiarch)
ifeq ($(GCCTARGET),)
GCCTARGET := $(shell $(CC) -dumpmachine)
endif
# TODO: download config.sub instead of hardcoding it in the repo
TARGET := $(shell ./config.sub $(GCCTARGET))

CSTD := -std=c17
WARNINGS := -Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
endif

ifneq (,$(findstring clang,$(CCVERSIONINFO)))
CLANGTARGET := $(shell $(CC) -dumpmachine)
TARGET := $(shell ./config.sub $(CLANGTARGET))

CSTD := -std=c17
WARNINGS := -Wall -Wextra -pedantic -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
endif

CFLAGS := 

ifndef TARGET
ifdef CC
$(error Compiler "$(CC)" is not supported)
else
$(error CC variable not set)
endif
endif
