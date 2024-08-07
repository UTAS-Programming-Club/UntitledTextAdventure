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

$(INCDIR)/zstd.h: third_party/zstd/lib/zstd.h | $(INCDIR)
	cp -r $< $@


$(INCDIR)/types.h: backend/types.in.h
	$(CPP) -P -C -nostdinc -o $@ $<

$(INCDIR)/types.json.h: backend/types.in.h
	$(CPP) -P -o $@ $< -D JSON

GameData.json: GameData.in.json $(INCDIR)/types.json.h
	$(CPP) -P -o $@ -xc $< -I $(INCDIR)


# TODO: Fix these being ignored on rebuild
# I had an issue with save.c not being rebuilt if types.in.h
# changed because it did not depend on types.h directly,
# despite depending on game.h which does depend on types.h.
backend/enemies.h: backend/game.h
backend/equipment.h: backend/game.h
backend/game.h: $(INCDIR)/arena.h $(INCDIR)/types.h # backend/equipment.h
backend/parser.h: backend/game.h $(INCDIR)/types.h
backend/save.h: backend/game.h
backend/screens.h: backend/game.h
backend/specialscreens.h: backend/game.h


# Objects
$(LIBDIR)/crossprint.o: backend/crossprint.c backend/crossprint.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/enemies.o: backend/enemies.c backend/enemies.h backend/equipment.h backend/game.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/equipment.o: backend/equipment.c backend/equipment.h backend/game.h $(INCDIR)/types.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/fileloading_frontend.o: backend/fileloading.c backend/fileloading.h frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS) -D FRONTEND

$(LIBDIR)/fileloading_printgamedata.o: backend/fileloading.c backend/fileloading.h frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/game.o: backend/game.c backend/equipment.h backend/game.h backend/parser.h backend/screens.h backend/save.h backend/specialscreens.h frontends/frontend.h $(INCDIR)/arena.h $(INCDIR)/types.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/parser.o: backend/parser.c backend/equipment.h backend/fileloading.h backend/game.h backend/parser.h backend/specialscreens.h backend/winresources.h frontends/frontend.h $(INCDIR)/cJSON.h $(INCDIR)/types.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/save.o: backend/save.c backend/equipment.h backend/game.h backend/save.h $(INCDIR)/arena.h $(INCDIR)/types.h $(INCDIR)/zstd.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/screens.o: backend/screens.c backend/game.h backend/parser.h backend/screens.h $(INCDIR)/arena.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/specialscreens.o: backend/specialscreens.c backend/game.h backend/parser.h backend/save.h backend/specialscreens.h $(INCDIR)/arena.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)


$(LIBDIR)/cJSON.o: third_party/cJSON/cJSON.c | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/libzstd.a: third_party/zstd/lib | $(LIBDIR)
	$(MAKE) -C $< libzstd.a ZSTD_NO_ASM=1
	cp $</libzstd.a $@

$(LIBDIR)/libzstd.so: third_party/zstd/lib | $(LIBDIR)
	$(MAKE) -C $< libzstd.so ZSTD_NO_ASM=1
	cp $</libzstd.so $@


$(LIBDIR)/jsonvalidator.o: tools/jsonvalidator.cpp $(INCDIR)/jsoncons $(INCDIR)/jsoncons_ext | $(LIBDIR)
	$(CXX) $(CXXSTD) $(CXXWARNINGS) -c -o $@ $< $(CXXFLAGS)

$(LIBDIR)/mapwatch.o: tools/mapwatch.c | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/printgamedata.o: tools/printgamedata.c backend/crossprint.h backend/parser.h frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)


$(LIBDIR)/cmdfrontend.o: frontends/cmdfrontend.c backend/crossprint.h backend/game.h frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/discordfrontend.o: frontends/discordfrontend.c frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS)

$(LIBDIR)/gdifrontend.o: frontends/gdifrontend.c backend/crossprint.h backend/game.h frontends/frontend.h | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -c -o $@ $< $(CFLAGS) $(GDICFLAGS)

$(LIBDIR)/winresources.o: frontends/winresources.rc backend/winresources.h GameData.json | $(LIBDIR)
	$(WINDRES) $< -o $@

$(LIBDIR)/game.so: $(filter-out $(LIBDIR)/libzstd.a,$(COMMONOBJS)) $(LIBDIR)/discordfrontend.o $(LIBDIR)/libzstd.so | $(LIBDIR)
	$(CC) $(CSTD) $(CWARNINGS) -o $@ $^ $(CFLAGS) -shared


# Executables
$(BINDIR)/jsonvalidator$(EXECSUFFIX): $(LIBDIR)/jsonvalidator.o | $(BINDIR)
	$(CXX) -o $(basename $@) $^ $(CXXFLAGS)
	$(call MAKEEXEC,$@,$(basename $@))

$(BINDIR)/mapwatch$(EXECSUFFIX): $(LIBDIR)/mapwatch.o
	$(CC) -o $(basename $@) $^ $(CFLAGS)
	$(call MAKEEXEC,$@,$(basename $@))

ifdef ISWINDOWS
$(BINDIR)/printgamedata$(EXECSUFFIX): $(LIBDIR)/cJSON.o $(LIBDIR)/crossprint.o $(LIBDIR)/equipment.o $(LIBDIR)/fileloading_printgamedata.o $(LIBDIR)/game.o $(LIBDIR)/parser.o $(LIBDIR)/printgamedata.o $(LIBDIR)/save.o $(LIBDIR)/screens.o $(LIBDIR)/specialscreens.o $(LIBDIR)/libzstd.a | $(BINDIR)
else # !ISWINDOWS
$(BINDIR)/printgamedata$(EXECSUFFIX): $(LIBDIR)/cJSON.o $(LIBDIR)/equipment.o $(LIBDIR)/fileloading_printgamedata.o $(LIBDIR)/game.o $(LIBDIR)/parser.o $(LIBDIR)/printgamedata.o $(LIBDIR)/save.o $(LIBDIR)/screens.o $(LIBDIR)/specialscreens.o $(LIBDIR)/libzstd.a | $(BINDIR)
endif # ISWINDOWS/!ISWINDOWS
	$(CC) -o $(basename $@) $^ $(CFLAGS) -lm
	$(call MAKEEXEC,$@,$(basename $@))


$(BINDIR)/cmdgame$(EXECSUFFIX): $(LIBDIR)/cmdfrontend.o $(COMMONOBJS) $(WINRESOURCES) | $(BINDIR)
	$(CC) -o $(basename $@) $^ $(CFLAGS) -lm
	$(call MAKEEXEC,$@,$(basename $@))

ifdef 0 # ISWINDOWS
$(BINDIR)/gdigame$(EXECSUFFIX): $(LIBDIR)/gdifrontend.o $(COMMONOBJS) $(WINRESOURCES) | $(BINDIR)
	$(CC) -o $(basename $@) $^ $(CFLAGS) $(GDICFLAGS)
	$(call MAKEEXEC,$@,$(basename $@))
else # !ISWINDOWS
$(BINDIR)/gdigame$(EXECSUFFIX):
endif # ISWINDOWS/!ISWINDOWS
