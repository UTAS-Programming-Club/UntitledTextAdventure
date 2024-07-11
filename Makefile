# CC_TARGETS = ~CLEAN_TARGETS
# TODO: Remove specific targets for debug and release and rely on $(MODE) being set
CLEAN_TARGETS   := clean distclean
CXX_TARGETS     := debug-tools tools
DEBUG_TARGETS   := debug debug-tools
RELEASE_TARGETS := discord release tools

.PHONY: $(CLEAN_TARGETS) $(CXX_TARGETS) $(DEBUG_TARGETS) $(RELEASE_TARGETS)

include build/compiler.mk
include build/mode.mk

OUTPUT := out/
OUTDIR := $(OUTPUT)/$(TARGET)/$(MODE)/
BINDIR := $(OUTDIR)/bin/
LIBDIR := $(OUTDIR)/lib/
INCDIR := $(OUTDIR)/include/

include build/targets.mk
include build/build.mk
