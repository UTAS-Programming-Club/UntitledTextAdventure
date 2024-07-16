COMMONOBJS := $(LIBDIR)/cJSON.o $(LIBDIR)/equipment.o $(LIBDIR)/fileloading_frontend.o $(LIBDIR)/game.o $(LIBDIR)/parser.o $(LIBDIR)/save.o $(LIBDIR)/screens.o $(LIBDIR)/specialscreens.o $(LIBDIR)/libzstd.a

CFLAGS += -I $(INCDIR)
CXXFLAGS += -I $(INCDIR)

ifdef ISCOSMO
CFLAGS += -mcosmo
CXXFLAGS += -mcosmo -fexceptions
COMMONOBJS += $(LIBDIR)/crossprint.o
EXECSUFFIX := .com
define MAKEEXEC =
$(APELINK) -l $(x86_64APEELF) -o $(1) $(2)
endef
else # !ISCOSMO
# mingw64 appends .exe if not present so copy will fail
define MAKEEXEC =
cp $(2) $(1) 2>/dev/null || true
endef

ifdef ISWINDOWS
CXXFLAGS += -Wa,-mbig-obj
GDICFLAGS := -municode -lgdi32
COMMONOBJS += $(LIBDIR)/crossprint.o
EXECSUFFIX := .exe

ifneq (,$(findstring release,$(MAKECMDGOALS)))
WINRESOURCES := $(LIBDIR)/winresources.o
ifndef WINDRES
$(error making release builds on windows requires the WINDRES environment variable to be set)
endif # WINDRES
endif # release build

endif # ISWINDOWS
endif # ISCOSMO/!ISCOSMO


# TODO: Support building specific frontends or tools
debug debug-tools: CFLAGS += -D _DEBUG -g
debug debug-tools: CXXFLAGS += -D _DEBUG -g
debug release: $(BINDIR)/cmdgame$(EXECSUFFIX) $(BINDIR)/gdigame$(EXECSUFFIX) GameData.json
discord: $(LIBDIR)/game.so GameData.json
tools debug-tools: $(BINDIR)/jsonvalidator$(EXECSUFFIX) $(BINDIR)/mapwatch$(EXECSUFFIX) $(BINDIR)/printgamedata$(EXECSUFFIX) GameData.json

clean:
	rm -r $(OUTPUT) GameData.json 2> /dev/null || true

distclean: clean
	$(MAKE) -C third_party/zstd/lib clean
