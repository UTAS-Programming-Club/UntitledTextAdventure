#define EMIT(text) text

#ifdef JSON
#define hash #

#define JSON_ENUM_START(name)
#define JSON_ENUM_ITEM(name, value) hash define name value
#define JSON_ENUM_END

#define C_EMIT(text)
#else
#define JSON_ENUM_START(name) enum name {
#define JSON_ENUM_ITEM(name, value) name = value,
#define JSON_ENUM_END };

#define C_EMIT(text) text
#endif

EMIT(#ifndef PCGAME_TYPES_H)
EMIT(#define PCGAME_TYPES_H)

C_EMIT(#include <stdint.h>)

// ScreenID is a uint16_t, [0, 65535]
// Must match indices in screen array in GameData.in.json
// Screen 0 is the default screen and is shown on startup
JSON_ENUM_START(Screen)
  JSON_ENUM_ITEM(MainMenuScreen,    0)
  JSON_ENUM_ITEM(GameScreen,        1)
  JSON_ENUM_ITEM(InvalidScreen, 65535)
JSON_ENUM_END

// InputOutcome is a uint16_t, [0, 65535]
JSON_ENUM_START(InputOutcome)
  JSON_ENUM_ITEM(InvalidInputOutcome,  0) // Do not use in json or use in screens.c
  JSON_ENUM_ITEM(GetNextOutputOutcome, 1) // Do not use in json or use in screens.c
  JSON_ENUM_ITEM(QuitGameOutcome,      2)
  JSON_ENUM_ITEM(GotoScreenOutcome,    3) // -> GetNextOutput, Needs newScreen field in the same screen's json entry
  JSON_ENUM_ITEM(GameGoNorthOutcome,   4) // -> GetNextOutput, Needs north in current room's json entry
  JSON_ENUM_ITEM(GameGoEastOutcome,    5) // -> GetNextOutput, Needs east in current room's json entry
  JSON_ENUM_ITEM(GameGoSouthOutcome,   6) // -> GetNextOutput, Needs south in current room's json entry
  JSON_ENUM_ITEM(GameGoWestOutcome,    7) // -> GetNextOutput, Needs west in current room's json entry
JSON_ENUM_END

// CustomScreenCode is a uint16_t, [0, 65535]
// Must match indices in CustomScreenCode array in specialscreens.c
JSON_ENUM_START(CustomScreenCode)
  JSON_ENUM_ITEM(MainMenuCustomScreenCode,    0)
  JSON_ENUM_ITEM(GameCustomScreenCode,        1)
  JSON_ENUM_ITEM(InvalidCustomScreenCode, 65535)
JSON_ENUM_END

// RoomType is a uint8_t, [0, 255]
JSON_ENUM_START(RoomType)
  JSON_ENUM_ITEM(EmptyRoomType,     0)
  JSON_ENUM_ITEM(InvalidRoomType, 255)
JSON_ENUM_END

// RoomType is a uint16_t, [0, 65535]
// Must match indices in rooms array in GameData.in.json
// Screen 0 is the default room and is shown when GameScreen becomes the current screen
C_EMIT(typedef uint_fast16_t RoomID;)
EMIT(#define InvalidRoomID 65535)

// TODO: Add enum for state vars

EMIT(#endif /* PCGAME_TYPES_H */)
