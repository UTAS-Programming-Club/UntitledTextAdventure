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

C_EMIT(#include <inttypes.h>)
C_EMIT(#include <stdint.h>)

// ScreenID is a uint16_t with [0, 65535)
// Must match indices in screen array in GameData.in.json
// Screen 0 is the default screen and is shown on startup
JSON_ENUM_START(Screen)
  JSON_ENUM_ITEM(MainMenuScreen,    0)
  JSON_ENUM_ITEM(GameScreen,        1)
  JSON_ENUM_ITEM(LoadScreen,        2)
  JSON_ENUM_ITEM(SaveScreen,        3)
  JSON_ENUM_ITEM(InvalidScreen, 65535)
JSON_ENUM_END

// InputOutcome is a uint16_t with (0, 65535]
JSON_ENUM_START(InputOutcome)
  // Can be given to frontend
  JSON_ENUM_ITEM(InvalidInputOutcome,  0) // Do not use in json or use in screens.c
  JSON_ENUM_ITEM(GetNextOutputOutcome, 1) // Do not use in json or use in screens.c
  JSON_ENUM_ITEM(QuitGameOutcome,      2)
  // Do not give to frontend
  JSON_ENUM_ITEM(GotoScreenOutcome,    3) // -> GetNextOutput, Needs newScreen field in the same screen's json entry
  JSON_ENUM_ITEM(GameGoNorthOutcome,   4) // -> GetNextOutput, Needs north in current room's json entry
  JSON_ENUM_ITEM(GameGoEastOutcome,    5) // -> GetNextOutput, Needs east in current room's json entry
  JSON_ENUM_ITEM(GameGoSouthOutcome,   6) // -> GetNextOutput, Needs south in current room's json entry
  JSON_ENUM_ITEM(GameGoWestOutcome,    7) // -> GetNextOutput, Needs west in current room's json entry
JSON_ENUM_END

// CustomScreenCode is a uint16_t with [0, 65535)
// Must match indices in CustomScreenCode array in specialscreens.c
JSON_ENUM_START(CustomScreenCode)
  JSON_ENUM_ITEM(MainMenuCustomScreenCode,    0)
  JSON_ENUM_ITEM(GameCustomScreenCode,        1)
  JSON_ENUM_ITEM(LoadCustomScreenCode,        2)
  JSON_ENUM_ITEM(SaveCustomScreenCode,        3)
  JSON_ENUM_ITEM(InvalidCustomScreenCode, 65535)
JSON_ENUM_END

// RoomType is a uint8_t with [0, 255)
JSON_ENUM_START(RoomType)
  JSON_ENUM_ITEM(EmptyRoomType,     0)
  JSON_ENUM_ITEM(InvalidRoomType, 255)
JSON_ENUM_END

// RoomCoord is a uint8_t with [0, FloorSize) <= [0, 255)
// Need to be able to add 1 safely for both printing on screen and for safely
// finding the next room. Same for subtracting 1 from 0, both give 255 which
// is defined to be invalid
EMIT(#define PRIRoomCoord PRIuFAST8)
EMIT(#define DefaultRoomCoordX (RoomCoord)0)
EMIT(#define DefaultRoomCoordY (RoomCoord)0)
EMIT(#define InvalidRoomCoord (RoomCoord)255)
C_EMIT(typedef uint_fast8_t RoomCoord;)
C_EMIT(typedef uint8_t RoomCoordSave;)

// TODO: Add enum for state vars

EMIT(#endif /* PCGAME_TYPES_H */)
