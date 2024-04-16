.PHONY: build clean

include compiler.mk

OUTPUT := out/
OUTDIR := $(OUTPUT)/$(TARGET)/
BINDIR := $(OUTDIR)/bin/
LIBDIR := $(OUTDIR)/lib/
INCDIR := $(OUTDIR)/include/

COMMONOBJS := $(LIBDIR)/alloc.o $(LIBDIR)/game.o $(LIBDIR)/strings.o
ifdef ISWINDOWS
COMMONOBJS += $(LIBDIR)/crossprint.o
endif

game: $(OUTPUT)/$(TARGET)/bin/cmdgame $(OUTPUT)/$(TARGET)/bin/gdigame
dev: $(OUTPUT)/$(TARGET)/bin/preptext

clean:
	rm -r $(OUTPUT) 2> /dev/null || true
	make -C third_party/b64.c clean

%/:
	mkdir -p $@

# Headers
$(INCDIR)/arena.h: third_party/arena/arena.h | $(INCDIR)
	cp $< $@

$(INCDIR)/b64.h: third_party/b64.c/b64.h | $(INCDIR)
	cp $< $@


# Objects
$(LIBDIR)/alloc.o: backend/alloc.c backend/alloc.h | $(INCDIR)/arena.h $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -I $(INCDIR)

$(LIBDIR)/game.o: backend/game.c backend/game.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/crossprint.o: shared/crossprint.c shared/crossprint.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/strings.o: shared/strings.c shared/strings.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/b64_buffer.o: third_party/b64.c/buffer.c third_party/b64.c/b64.h | $(LIBDIR)
	make -C third_party/b64.c buffer.o
	mv third_party/b64.c/buffer.o $@

$(LIBDIR)/b64_encode.o: third_party/b64.c/encode.c third_party/b64.c/b64.h | $(LIBDIR)
	make -C third_party/b64.c encode.o
	mv third_party/b64.c/encode.o $@

$(LIBDIR)/preptext.o: tools/preptext.c | $(INCDIR)/b64.h $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -I $(INCDIR)

$(LIBDIR)/cmdfrontend.o: frontends/cmdfrontend.c frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/gdifrontend.o: frontends/gdifrontend.c frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -municode


# Executables
$(OUTPUT)/$(TARGET)/bin/preptext: $(LIBDIR)/b64_buffer.o $(LIBDIR)/b64_encode.o $(LIBDIR)/preptext.o $(LIBDIR)/strings.o | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS)

$(OUTPUT)/$(TARGET)/bin/cmdgame: $(LIBDIR)/cmdfrontend.o $(COMMONOBJS) | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS) -lm

ifdef ISWINDOWS
$(OUTPUT)/$(TARGET)/bin/gdigame: $(LIBDIR)/gdifrontend.o $(COMMONOBJS) | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS) -municode -l gdi32
else
$(OUTPUT)/$(TARGET)/bin/gdigame:
endif
