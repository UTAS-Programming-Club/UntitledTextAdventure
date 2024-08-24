#define hash #
#define EMIT(text) text

// TODO: Split into common, backend and json files

#ifdef JSON
#define JSON_ENUM_START(name)
#define JSON_ENUM_ITEM(name, value) hash define name value
#define JSON_ENUM_END

#define VALUE_EMIT(type, name, value) hash define name value
#define SAVED_INTEGRAL_TYPE_EMIT(type, size, name)
#define C_EMIT(text)
#else
#define JSON_ENUM_START(name) enum name {
#define JSON_ENUM_ITEM(name, value) name = value,
#define JSON_ENUM_END };

#define VALUE_EMIT(type, name, value) hash define name (type)(value)
#define SAVED_INTEGRAL_TYPE_EMIT(type, size, name) \
typedef type##_fast##size##_t name; \
typedef type##size##_t name##Save;
#define C_EMIT(text) text
#endif

EMIT(#ifndef PCGAME_TYPES_H)
EMIT(#define PCGAME_TYPES_H)

C_EMIT(#include <inttypes.h>)
C_EMIT(#include <stdint.h>)

// TODO: Restrict operators on IDs to prevent modifying them via anything other than assignment
// TODO: Restrict operators on stats to enforce min and max

// ScreenID is a uint16_t with [0, 65535)
// Must match indices in screen array in GameData.in.json
// Screen 0 is the default screen and is shown on startup
JSON_ENUM_START(Screen)
  JSON_ENUM_ITEM(MainMenuScreen,        0)
  JSON_ENUM_ITEM(GameScreen,            1)
  JSON_ENUM_ITEM(PlayerStatsScreen,     2)
  JSON_ENUM_ITEM(LoadScreen,            3)
  JSON_ENUM_ITEM(SaveScreen,            4)
  JSON_ENUM_ITEM(PlayerEquipmentScreen, 5)
  JSON_ENUM_ITEM(CombatScreen,          6)
  JSON_ENUM_ITEM(InvalidScreen,     65535)
JSON_ENUM_END

// ScreenType is a uint16_t with (0, 255]
JSON_ENUM_START(ScreenInputType)
  JSON_ENUM_ITEM(InvalidScreenInputType, 0)
  JSON_ENUM_ITEM(ButtonScreenInputType,  1)
  JSON_ENUM_ITEM(TextScreenInputType,    2)
  JSON_ENUM_ITEM(NoneScreenInputType,    3)
JSON_ENUM_END

// InputOutcome is a uint16_t with (0, 65535]
JSON_ENUM_START(InputOutcome)
  // Can be given to frontend
  JSON_ENUM_ITEM(InvalidInputOutcome,      0) // Do not use in json or screens.c
  JSON_ENUM_ITEM(GetNextOutputOutcome,     1) // Do not use in json or screens.c
  JSON_ENUM_ITEM(QuitGameOutcome,          2)
  // Do not give to frontend
  JSON_ENUM_ITEM(GotoScreenOutcome,        3) // -> GetNextOutput, Needs newScreen field in the same button's json entry
  JSON_ENUM_ITEM(GameGoNorthOutcome,       4) // -> GetNextOutput, Needs next room to exist in json
  JSON_ENUM_ITEM(GameGoEastOutcome,        5) // -> GetNextOutput, Needs next room to exist in json
  JSON_ENUM_ITEM(GameGoSouthOutcome,       6) // -> GetNextOutput, Needs next room to exist in json
  JSON_ENUM_ITEM(GameGoWestOutcome,        7) // -> GetNextOutput, Needs next room to exist in json
  JSON_ENUM_ITEM(GameHealthChangeOutcome,  8) // -> GetNextOutput, Needs percentageChance and healthChange in current room's json entry
  JSON_ENUM_ITEM(GameSwapEquipmentOutcome, 9) // -> GetNextOutput, Needs equipmentSlot field in the same button's json entry
  JSON_ENUM_ITEM(GameOpenChestOutcome,    10) // -> GetNextOutput, Needs ID of item contained in current room's json entry
  JSON_ENUM_ITEM(GameFightEnemiesOutcome, 11) // -> GetNextOutput
JSON_ENUM_END

// CustomScreenCode is a uint16_t with [0, 65535)
// Must match indices in CustomScreenCode array in specialscreens.c
JSON_ENUM_START(CustomScreenCode)
  JSON_ENUM_ITEM(MainMenuCustomScreenCode,        0)
  JSON_ENUM_ITEM(GameCustomScreenCode,            1)
  JSON_ENUM_ITEM(PlayerStatsCustomScreenCode,     2)
  JSON_ENUM_ITEM(SaveCustomScreenCode,            3)
  JSON_ENUM_ITEM(PlayerEquipmentCustomScreenCode, 4)
  JSON_ENUM_ITEM(CombatCustomScreenCode,          5)
  JSON_ENUM_ITEM(InvalidCustomScreenCode,     65535)
JSON_ENUM_END

// RoomType is a uint8_t with [0, 255)
JSON_ENUM_START(RoomType)
  JSON_ENUM_ITEM(EmptyRoomType,          0)
  // TODO: Change to general stat change room type, support more than one stat?
  JSON_ENUM_ITEM(HealthChangeRoomType,   1)
  // TODO: Readd stat check room type
  JSON_ENUM_ITEM(CustomChestRoomType,    2)
  JSON_ENUM_ITEM(CombatRoomType,         3)
  JSON_ENUM_ITEM(InvalidRoomType,      255)
JSON_ENUM_END


// RoomCoord is a uint8_t with [0, FloorSize) <= [0, 255)
// Need to be able to add 1 safely for both printing on screen and for safely
// finding the next room. Same for subtracting 1 from 0, both give 255 which
// is defined to be invalid
C_EMIT(#define PRIRoomCoord PRIuFAST8)
// TODO: Make these c only?
VALUE_EMIT(RoomCoord, DefaultRoomCoordX,  0)
VALUE_EMIT(RoomCoord, DefaultRoomCoordY,  0)
VALUE_EMIT(RoomCoord, InvalidRoomCoord, 255)
SAVED_INTEGRAL_TYPE_EMIT(uint, 8, RoomCoord)


// EntityStat is a uint8_t with [0, 100]
C_EMIT(#define PRIEntityStat PRIuFAST8)
// TODO: Use json loaded default stats when possible instead of these?
VALUE_EMIT(EntityStat, MinimumEntityStat,   0)
VALUE_EMIT(EntityStat, MaximumEntityStat, 100)
SAVED_INTEGRAL_TYPE_EMIT(uint, 8, EntityStat)

// EntityStatDiff is a int8_t with [-100, 100]
C_EMIT(#define PRIEntityStatDiff PRIdFAST8)
VALUE_EMIT(EntityStatDiff, MinimumEntityStatDiff, -100)
VALUE_EMIT(EntityStatDiff, MaximumEntityStatDiff,  100)
VALUE_EMIT(EntityStatDiff, InvalidEntityStatDiff, INT_FAST8_MAX)
C_EMIT(typedef int_fast8_t EntityStatDiff;)

// TODO: Change json to use equipmentType once the sword slots are merged
// EquipmentType is a uint8_t with [0, EquipmentTypeCount)
C_EMIT(#define EquipmentTypeCount (EquipmentType)7)
C_EMIT(typedef uint_fast8_t EquipmentType;)

// EquipmentID is a uint8_t with [0, 62]
// With 7 types of items, this gives 9 items per type
// Equipment types: helmets, chest pieces, gloves, pants, boots, primary weapon, secondary weapon
C_EMIT(#define EquipmentPerTypeCount 9)
C_EMIT(#define EquipmentCount EquipmentTypeCount * EquipmentPerTypeCount)
VALUE_EMIT(EquipmentID, InvalidEquipmentID, UINT_FAST8_MAX)
VALUE_EMIT(EquipmentIDSave, InvalidEquipmentIDSave, 0)
SAVED_INTEGRAL_TYPE_EMIT(uint, 8, EquipmentID)


// EnemyAttackType is a uint8_t with [1, 2]
JSON_ENUM_START(EnemyAttackType)
  JSON_ENUM_ITEM(InvalidEnemyAttackType, 0)
  JSON_ENUM_ITEM(PhysEnemyAttackType,    1)
  JSON_ENUM_ITEM(MagEnemyAttackType,     2)
JSON_ENUM_END

C_EMIT(#define CombatEventInfoCount (size_t)8)
// CombatEventCauser is a uint8_t with [1, 3]
JSON_ENUM_START(CombatEventCause)
  JSON_ENUM_ITEM(InvalidCombatEventCause, 0)
  JSON_ENUM_ITEM(UnusedCombatEventCause,  1)
  JSON_ENUM_ITEM(PlayerCombatEventCause,  2)
  JSON_ENUM_ITEM(EnemyCombatEventCause,   3)
JSON_ENUM_END

// TODO: Add enum for state vars

EMIT(#endif /* PCGAME_TYPES_H */)
