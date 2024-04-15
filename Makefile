.PHONY: build clean

include compiler.mk

OUTPUT := out/
OUTDIR := $(OUTPUT)/$(TARGET)/
BINDIR := $(OUTDIR)/bin/
LIBDIR := $(OUTDIR)/lib/
INCDIR := $(OUTDIR)/include/

COMMONOBJS := $(LIBDIR)/alloc.o $(LIBDIR)/game.o
ifdef ISWINDOWS
COMMONOBJS += $(LIBDIR)/crossprint.o $(LIBDIR)/strings.o
endif

build: $(OUTPUT)/$(TARGET)/bin/cmdgame $(OUTPUT)/$(TARGET)/bin/gdigame

clean:
	rm -r $(OUTPUT) 2> /dev/null || true

%/:
	mkdir -p $@

# Headers
$(INCDIR)/arena.h: third_party/arena/arena.h | $(INCDIR)
	cp $< $@


# Objects
$(LIBDIR)/alloc.o: backend/alloc.c backend/alloc.h | $(INCDIR)/arena.h $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -I $(INCDIR)

$(LIBDIR)/game.o: backend/game.c backend/game.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -I $(INCDIR)

$(LIBDIR)/crossprint.o: shared/crossprint.c shared/crossprint.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -I $(INCDIR)

$(LIBDIR)/strings.o: shared/strings.c shared/strings.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -I $(INCDIR)

$(LIBDIR)/cmdfrontend.o: frontends/cmdfrontend.c frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -I $(INCDIR)

$(LIBDIR)/gdifrontend.o: frontends/gdifrontend.c frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(WARNINGS) -c -o $@ $< $(CFLAGS) -I $(INCDIR) -municode


# Executables
$(OUTPUT)/$(TARGET)/bin/cmdgame: $(LIBDIR)/cmdfrontend.o $(COMMONOBJS) | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS)

ifdef ISWINDOWS
$(OUTPUT)/$(TARGET)/bin/gdigame: $(LIBDIR)/gdifrontend.o $(COMMONOBJS) | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS) -municode -l gdi32
else
$(OUTPUT)/$(TARGET)/bin/gdigame:
endif
