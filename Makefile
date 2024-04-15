.PHONY: build clean

include compiler.mk

OUTPUT := out/

build: $(OUTPUT)/$(TARGET)/bin/cmdgame $(OUTPUT)/$(TARGET)/bin/gdigame

clean:
	rm -r $(OUTPUT) 2> /dev/null || true

%/:
	mkdir -p $@

$(OUTPUT)/$(TARGET)/bin/cmdgame: game.c cmdfrontend.c crossprint.c strings.c | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS)

ifdef ISWINDOWS
$(OUTPUT)/$(TARGET)/bin/gdigame: game.c gdifrontend.c crossprint.c strings.c | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS) -municode -l gdi32
else
$(OUTPUT)/$(TARGET)/bin/gdigame:
endif