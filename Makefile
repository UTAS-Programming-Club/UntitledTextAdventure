.PHONY: build clean

ifneq (,$(findstring tools,$(MAKECMDGOALS)))
NEEDCXX := TRUE
endif # tools build

include compiler.mk

OUTPUT := out/
OUTDIR := $(OUTPUT)/$(TARGET)/
BINDIR := $(OUTDIR)/bin/
LIBDIR := $(OUTDIR)/lib/
INCDIR := $(OUTDIR)/include/

COMMONOBJS := $(LIBDIR)/cJSON.o $(LIBDIR)/fileloading_frontend.o $(LIBDIR)/game.o $(LIBDIR)/parser.o $(LIBDIR)/screens.o $(LIBDIR)/specialscreens.o

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
GDICFLAGS := -municode -l gdi32
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
debug: CFLAGS += -D _DEBUG -g
debug: CXXFLAGS += -D _DEBUG -g
debug release: $(BINDIR)/cmdgame$(EXECSUFFIX) $(BINDIR)/gdigame$(EXECSUFFIX) GameData.json
discord: $(LIBDIR)/game.so GameData.json
tools: $(BINDIR)/printgamedata$(EXECSUFFIX) $(BINDIR)/jsonvalidator$(EXECSUFFIX) GameData.json

clean:
	rm -r $(OUTPUT) GameData.json backend/types.h backend/types.json.h 2> /dev/null || true

%/:
	mkdir -p $@


# Headers
$(INCDIR)/arena.h: third_party/arena/arena.h | $(INCDIR)
	cp $< $@

$(INCDIR)/cJSON.h: third_party/cJSON/cJSON.h | $(INCDIR)
	cp $< $@

$(INCDIR)/jsoncons: third_party/jsoncons/include/jsoncons | $(INCDIR)
	cp -r $< $@

$(INCDIR)/jsoncons_ext: third_party/jsoncons/include/jsoncons_ext | $(INCDIR)
	cp -r $< $@


backend/game.h: $(INCDIR)/arena.h backend/types.h

backend/types.h: backend/types.in.h
	$(CPP) -P -C -nostdinc -o $@ $<

backend/types.json.h: backend/types.in.h
	$(CPP) -P -o $@ $< -D JSON

GameData.json: GameData.in.json backend/types.json.h
	$(CPP) -P -o $@ -xc $<


# Objects
$(LIBDIR)/game.o: backend/game.c backend/game.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/screens.o: backend/screens.c backend/screens.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/specialscreens.o: backend/specialscreens.c backend/specialscreens.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)


$(LIBDIR)/crossprint.o: shared/crossprint.c shared/crossprint.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/fileloading_frontend.o: shared/fileloading.c shared/fileloading.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS) -D FRONTEND

$(LIBDIR)/fileloading_printgamedata.o: shared/fileloading.c shared/fileloading.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/parser.o: shared/parser.c backend/game.h shared/parser.h $(INCDIR)/cJSON.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS) -D BACKEND


$(LIBDIR)/cJSON.o: third_party/cJSON/cJSON.c third_party/cJSON/cJSON.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)


$(LIBDIR)/printgamedata.o: tools/printgamedata.c | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/jsonvalidator.o: tools/jsonvalidator.cpp $(INCDIR)/jsoncons $(INCDIR)/jsoncons_ext | $(LIBDIR)
	$(CXX) $(CXXSTD) $(CXXWARNINGS) -c -o $@ $< $(CXXFLAGS)


$(LIBDIR)/cmdfrontend.o: frontends/cmdfrontend.c frontends/frontend.h backend/game.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/gdifrontend.o: frontends/gdifrontend.c frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS) $(GDICFLAGS)

$(LIBDIR)/winresources.o: frontends/winresources.rc shared/winresources.h GameData.json | $(LIBDIR)
	$(WINDRES) $< -o $@

$(LIBDIR)/game.so: $(COMMONOBJS) | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -o $@ $^ $(CFLAGS) -shared


# Executables
ifdef ISWINDOWS
$(BINDIR)/printgamedata$(EXECSUFFIX): $(LIBDIR)/cJSON.o $(LIBDIR)/crossprint.o $(LIBDIR)/fileloading_printgamedata.o $(LIBDIR)/parser.o $(LIBDIR)/printgamedata.o | $(BINDIR)
else # !ISWINDOWS
$(BINDIR)/printgamedata$(EXECSUFFIX): $(LIBDIR)/cJSON.o $(LIBDIR)/fileloading_printgamedata.o $(LIBDIR)/parser.o $(LIBDIR)/printgamedata.o | $(BINDIR)
endif # ISWINDOWS/!ISWINDOWS
	$(CC) -o $(basename $@) $^ $(CFLAGS)
	$(call MAKEEXEC,$@,$(basename $@))

$(BINDIR)/jsonvalidator$(EXECSUFFIX): $(LIBDIR)/jsonvalidator.o | $(BINDIR)
	$(CXX) -o $(basename $@) $^ $(CXXFLAGS)
	$(call MAKEEXEC,$@,$(basename $@))

$(BINDIR)/cmdgame$(EXECSUFFIX): $(LIBDIR)/cmdfrontend.o $(COMMONOBJS) $(WINRESOURCES) | $(BINDIR)
	$(CC) -o $(basename $@) $^ $(CFLAGS) -lm
	$(call MAKEEXEC,$@,$(basename $@))

ifdef ISWINDOWS
$(BINDIR)/gdigame$(EXECSUFFIX): $(LIBDIR)/gdifrontend.o $(COMMONOBJS) $(WINRESOURCES) | $(BINDIR)
	$(CC) -o $(basename $@) $^ $(CFLAGS) $(GDICFLAGS)
	$(call MAKEEXEC,$@,$(basename $@))
else # !ISWINDOWS
$(BINDIR)/gdigame$(EXECSUFFIX):
endif # ISWINDOWS/!ISWINDOWS
