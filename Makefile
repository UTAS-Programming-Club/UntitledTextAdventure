.PHONY: build clean

include compiler.mk 

OUTPUT := out/

build: $(OUTPUT)/$(TARGET)/bin/game

clean:
	rm -r $(OUTPUT) 2> /dev/null || true

%/:
	mkdir -p $@

$(OUTPUT)/$(TARGET)/bin/game: main.c | $(OUTPUT)/$(TARGET)/bin/
	$(CC) $(CSTD) $(WARNINGS) -o $@ $^ $(CFLAGS)
