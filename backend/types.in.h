#define EMIT(text) text

#ifdef JSON
#define hash #

#define JSON_ENUM_START(name)
#define JSON_ENUM_ITEM(name, value) hash define name value
#define JSON_ENUM_END
#else
#define JSON_ENUM_START(name) enum name {
#define JSON_ENUM_ITEM(name, value) name = value,
#define JSON_ENUM_END };
#endif

EMIT(#ifndef PCGAME_TYPES_H)
EMIT(#define PCGAME_TYPES_H)

// ScreenID is a uint16_t, [0, 65535]
// Must match index in screen array in GameData.in.json
JSON_ENUM_START(ScreenID)
  JSON_ENUM_ITEM(MainMenu, 0)
  JSON_ENUM_ITEM(Test1,    1)
  JSON_ENUM_ITEM(Test2,    2)
  JSON_ENUM_ITEM(Test3,    3)
  JSON_ENUM_ITEM(InvalidScreenID, 65535)
JSON_ENUM_END

// GameInputOutcome is a uint16_t, [0, 65535]
JSON_ENUM_START(InputOutcome)
  JSON_ENUM_ITEM(InvalidInputOutcome, 0) // Do not use in json or use in screens.c
  JSON_ENUM_ITEM(GetNextOutput, 1)       // Do not use in json or use in screens.c
  JSON_ENUM_ITEM(QuitGame,      2)
  JSON_ENUM_ITEM(GotoScreen,    3)       // -> GetNextOutput, Needs newScreen field from json
JSON_ENUM_END

// CustomScreenID is a uint16_t, [0, 65535]
// Must match index in CustomScreen array in specialscreens.c
JSON_ENUM_START(CustomScreenID)
  JSON_ENUM_ITEM(MainMenuCustomScreen, 0)
  JSON_ENUM_ITEM(InvalidCustomScreenID, 65535)
JSON_ENUM_END

EMIT(#endif /* PCGAME_TYPES_H */)
