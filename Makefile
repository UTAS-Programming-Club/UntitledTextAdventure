.PHONY: build clean

include compiler.mk

OUTPUT := out/
OUTDIR := $(OUTPUT)/$(TARGET)/
BINDIR := $(OUTDIR)/bin/
LIBDIR := $(OUTDIR)/lib/
INCDIR := $(OUTDIR)/include/

COMMONOBJS := $(LIBDIR)/alloc.o $(LIBDIR)/b64_buffer.o $(LIBDIR)/b64_decode.o $(LIBDIR)/base64_backend.o $(LIBDIR)/cJSON.o $(LIBDIR)/fileloading_frontend.o $(LIBDIR)/game.o $(LIBDIR)/parser.o $(LIBDIR)/screens.o $(LIBDIR)/strings.o

ifdef ISCOSMO
CFLAGS += -mcosmo
COMMONOBJS += $(LIBDIR)/crossprint.o
else
ifdef ISWINDOWS
GDICFLAGS := -municode -l gdi32
COMMONOBJS += $(LIBDIR)/crossprint.o

ifneq (,$(findstring release,$(MAKECMDGOALS)))
WINRESOURCES := $(LIBDIR)/winresources.o
ifndef WINDRES
$(error making release builds on windows requires the WINDRES environment variable to be set)
endif
endif

endif
endif

# Makes valgrind work better
ifneq (,$(findstring debug,$(MAKECMDGOALS)))
CFLAGS += -g
endif

debug: CFLAGS += -D _DEBUG
debug release: $(OUTPUT)/$(TARGET)/bin/cmdgame $(OUTPUT)/$(TARGET)/bin/gdigame
tools: $(OUTPUT)/$(TARGET)/bin/preptext $(OUTPUT)/$(TARGET)/bin/printgamedata

clean:
	rm -r $(OUTPUT) 2> /dev/null || true
	make -C third_party/b64.c clean $(SUBMAKESHELL) $(SUBMAKEPATH)

%/:
	mkdir -p $@


# Headers
$(INCDIR)/arena.h: third_party/arena/arena.h | $(INCDIR)
	cp $< $@

$(INCDIR)/b64.h: third_party/b64.c/b64.h | $(INCDIR)
	cp $< $@

$(INCDIR)/cJSON.h: third_party/cJSON/cJSON.h | $(INCDIR)
	cp $< $@


# Objects
$(LIBDIR)/alloc.o: backend/alloc.c backend/alloc.h $(INCDIR)/arena.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -I $(INCDIR)

$(LIBDIR)/game.o: backend/game.c backend/game.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/screens.o: backend/screens.c backend/screens.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)


$(LIBDIR)/base64_backend.o: shared/base64.c shared/base64.h shared/strings.h $(INCDIR)/b64.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -D BACKEND -I $(INCDIR)

$(LIBDIR)/base64_preptext.o: shared/base64.c shared/base64.h shared/strings.h $(INCDIR)/b64.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -D PREPTEXT -I $(INCDIR)

$(LIBDIR)/crossprint.o: shared/crossprint.c shared/crossprint.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/fileloading_frontend.o: shared/fileloading.c shared/fileloading.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -D FRONTEND

$(LIBDIR)/fileloading_printgamedata.o: shared/fileloading.c shared/fileloading.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/parser.o: shared/parser.c shared/parser.h $(INCDIR)/b64.h $(INCDIR)/cJSON.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -D BACKEND -I $(INCDIR)

$(LIBDIR)/strings.o: shared/strings.c shared/strings.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)


$(LIBDIR)/b64_buffer.o: third_party/b64.c/buffer.c third_party/b64.c/b64.h | $(LIBDIR)
	make -C third_party/b64.c buffer.o $(SUBMAKESHELL) $(SUBMAKEPATH) $(SUBMAKECC)
	mv third_party/b64.c/buffer.o $@

$(LIBDIR)/b64_encode.o: third_party/b64.c/encode.c third_party/b64.c/b64.h | $(LIBDIR)
	make -C third_party/b64.c encode.o $(SUBMAKESHELL) $(SUBMAKEPATH) $(SUBMAKECC)
	mv third_party/b64.c/encode.o $@

$(LIBDIR)/b64_decode.o: third_party/b64.c/decode.c third_party/b64.c/b64.h | $(LIBDIR)
	make -C third_party/b64.c decode.o $(SUBMAKESHELL) $(SUBMAKEPATH) $(SUBMAKECC)
	mv third_party/b64.c/decode.o $@


$(LIBDIR)/cJSON.o: third_party/cJSON/cJSON.c third_party/cJSON/cJSON.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)


$(LIBDIR)/preptext.o: tools/preptext.c $(INCDIR)/b64.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -D PREPTEXT -I $(INCDIR)

$(LIBDIR)/printgamedata.o: tools/printgamedata.c | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)


$(LIBDIR)/cmdfrontend.o: frontends/cmdfrontend.c frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/gdifrontend.o: frontends/gdifrontend.c frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) $(GDICFLAGS)

$(LIBDIR)/winresources.o: frontends/winresources.rc shared/winresources.h GameData.json | $(LIBDIR)
	$(WINDRES) $< -o $@


# Executables
$(OUTPUT)/$(TARGET)/bin/preptext: $(LIBDIR)/b64_buffer.o $(LIBDIR)/b64_encode.o $(LIBDIR)/base64_preptext.o $(LIBDIR)/preptext.o $(LIBDIR)/strings.o | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS)

$(OUTPUT)/$(TARGET)/bin/printgamedata: $(LIBDIR)/b64_buffer.o $(LIBDIR)/b64_decode.o $(LIBDIR)/base64_backend.o $(LIBDIR)/cJSON.o $(LIBDIR)/fileloading_printgamedata.o $(LIBDIR)/parser.o $(LIBDIR)/printgamedata.o | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS)

$(OUTPUT)/$(TARGET)/bin/cmdgame: $(LIBDIR)/cmdfrontend.o $(COMMONOBJS) $(WINRESOURCES) | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS) -lm

ifdef ISWINDOWS
$(OUTPUT)/$(TARGET)/bin/gdigame: $(LIBDIR)/gdifrontend.o $(COMMONOBJS) $(WINRESOURCES) | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS) $(GDICFLAGS)
else
$(OUTPUT)/$(TARGET)/bin/gdigame:
endif
