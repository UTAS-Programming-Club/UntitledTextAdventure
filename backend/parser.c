//#define DEBUG_STATE

#include <cJSON.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <types.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "../frontends/frontend.h"
#include "fileloading.h"
#include "game.h"
#include "parser.h"
#include "winresources.h"

#define CAT_(a, b) a ## b
#define CAT(a, b) CAT_(a, b)

#define JSON_GETJSONOBJECTERROR(var, obj, name, err) \
  var = cJSON_GetObjectItemCaseSensitive(obj, name); \
  if (!cJSON_IsObject(var)) { \
    return err; \
  }

#define JSON_GETJSONARRAYERROR(var, obj, name, err) \
  var = cJSON_GetObjectItemCaseSensitive(obj, name); \
  if (!cJSON_IsArray(var)) { \
    return err; \
  }

#define JSON_GETJSONARRAYITEMOBJERROR(var, arr, idx, err) \
  var = cJSON_GetArrayItem(arr, idx); \
  if (!cJSON_IsObject(var)) { \
    return false; \
  }

#define JSON_GETJSONARRAYITEMNUMBERERROR(var, arr, idx, err) \
  cJSON *CAT(json, __LINE__) = cJSON_GetArrayItem(arr, idx); \
  if (!cJSON_IsNumber(CAT(json, __LINE__))) { \
    return err; \
  } \
  var = cJSON_GetNumberValue(CAT(json, __LINE__));

#define JSON_GETSTRINGVALUEERROR(var, obj, name, err) \
  cJSON *CAT(json, __LINE__) = cJSON_GetObjectItemCaseSensitive(obj, name); \
  var = cJSON_GetStringValue(CAT(json, __LINE__)); \
  if (!var) { \
    return err; \
  }

#define JSON_GETNUMBERVALUEERROR(var, obj, name, err) \
  cJSON *CAT(json, __LINE__) = cJSON_GetObjectItemCaseSensitive(obj, name); \
  if (!cJSON_IsNumber(CAT(json, __LINE__))) { \
    return err; \
  } \
  var = cJSON_GetNumberValue(CAT(json, __LINE__));

static void *Data = NULL;
static cJSON *GameData = NULL;
static const double invalidOptNumberVal = -1.0;

static inline double cJSON_GetOptNumberValue(const cJSON *const object, const char *const string,
                                             double missingVal, double invalidVal) {
  cJSON *jsonVar = cJSON_GetObjectItemCaseSensitive(object, string);
  if (!jsonVar) {
    return missingVal;
  }

  if (!cJSON_IsNumber(jsonVar)) {
    return invalidVal;
  }

  return cJSON_GetNumberValue(jsonVar);
}


static bool GetGameRoomData(cJSON *, struct RoomInfo *);
static bool GetGameEquipmentItemData(cJSON *, struct EquipmentInfo *);


bool LoadGameData(char *path) {
  if (GameData) {
    return true;
  }

  if (!path) {
    return false;
  }

  size_t size;
  if (!LoadFile(path, &size, &Data, GAMEDATA, GAMEDATA_RESTYPE)) {
    return false;
  }

  GameData = cJSON_ParseWithLength(Data, size);
  if (!GameData) {
    UnloadFile(Data, GAMEDATA, GAMEDATA_RESTYPE);
    // TODO: Report erroring line and column numbers
    // TODO: Show erroring line
    PrintError("Unable to load %s, error in parsing at position %td", path, cJSON_GetErrorPtr() - (char *)Data);
    return false;
  }

  return true;
}

void UnloadGameData(void) {
  if (GameData) {
    cJSON_Delete(GameData);
    GameData = NULL;
  }

  if (Data) {
    UnloadFile(Data, GAMEDATA, GAMEDATA_RESTYPE);
    Data = NULL;
  }
}


char *LoadGameName(void) {
  if (!GameData) {
    return NULL;
  }

  char *name = NULL;
  JSON_GETSTRINGVALUEERROR(name, GameData, "name", NULL);

  return name;
}

bool LoadDefaultPlayerStats(struct PlayerInfo *playerStats) {
  if (!GameData || !playerStats) {
    return false;
  }

  cJSON *jsonDefaultStats;
  JSON_GETJSONOBJECTERROR(jsonDefaultStats, GameData, "defaultStats", false);

  JSON_GETNUMBERVALUEERROR(playerStats->health, jsonDefaultStats, "health", false);
  JSON_GETNUMBERVALUEERROR(playerStats->stamina, jsonDefaultStats, "stamina", false);
  JSON_GETNUMBERVALUEERROR(playerStats->physAtk, jsonDefaultStats, "physAtk", false);
  JSON_GETNUMBERVALUEERROR(playerStats->magAtk, jsonDefaultStats, "magAtk", false);
  JSON_GETNUMBERVALUEERROR(playerStats->physDef, jsonDefaultStats, "physDef", false);
  JSON_GETNUMBERVALUEERROR(playerStats->magDef, jsonDefaultStats, "magDef", false);

  return true;
}

bool LoadGameRooms(uint_fast8_t *floorSize, struct RoomInfo **rooms) {
  if (!GameData || !floorSize || !rooms) {
    return false;
  }

  cJSON *jsonRooms;
  JSON_GETJSONOBJECTERROR(jsonRooms, GameData, "rooms", false);

  JSON_GETNUMBERVALUEERROR(*floorSize, jsonRooms, "floorSize", false);

  size_t floorRoomCount = *floorSize * *floorSize;
  *rooms = calloc(floorRoomCount, sizeof **rooms);
  if (!*rooms) {
    return false;
  }

  for (size_t i = 0; i < floorRoomCount; ++i) {
    (*rooms)[i].type = InvalidRoomType;
  }

  cJSON *jsonRoomsArray;
  JSON_GETJSONARRAYERROR(jsonRoomsArray, jsonRooms, "roomsArray", false);

  // cJSON_ArrayForEach uses int for idx, likely fine as INT_MAX >= 2^15 - 1
  cJSON *jsonRoom;
  cJSON_ArrayForEach(jsonRoom, jsonRoomsArray) {
    cJSON *jsonPosition;
    JSON_GETJSONARRAYERROR(jsonPosition, jsonRoom, "position", false);

    RoomCoord x, y;
    JSON_GETJSONARRAYITEMNUMBERERROR(x, jsonPosition, 0, false);
    JSON_GETJSONARRAYITEMNUMBERERROR(y, jsonPosition, 1, false);

    uint_fast16_t idx = *floorSize * y + x;
    struct RoomInfo *room = &(*rooms)[idx];
    room->x = x;
    room->y = y;
    if (!GetGameRoomData(jsonRoom, room)) {
      return false;
    }
  }

  return true;
}

bool LoadGameEquipment(uint_fast8_t *equipmentCount, struct EquipmentInfo **equipment) {
  if (!GameData || !equipmentCount || !equipment) {
    return false;
  }

  cJSON *jsonEquipment;
  JSON_GETJSONARRAYERROR(jsonEquipment, GameData, "equipment", false);

  *equipmentCount = cJSON_GetArraySize(jsonEquipment);

  *equipment = calloc(*equipmentCount, sizeof **equipment);
  if (!*equipment) {
    return false;
  }

  struct EquipmentInfo *currentItem = *equipment;
  uint_fast8_t i = 0;

  // cJSON_ArrayForEach uses int for idx, likely fine as INT_MAX >= 2^15 - 1
  cJSON *jsonItem;
  cJSON_ArrayForEach(jsonItem, jsonEquipment) {
    currentItem->id = i;
    if (!GetGameEquipmentItemData(jsonItem, currentItem)) {
      return false;
    }

    ++i;
    ++currentItem;
  }

  return true;
}


// Currently only integers are supported
// TODO: Support floating point
// TODO: Support loading data from "save", the plan is to use a password system so no actual saves per se
// Must be freed at the end of the program
unsigned char *InitGameState(void) {
  if (!GameData) {
    return NULL;
  }

  cJSON *jsonStateVars;
  JSON_GETJSONARRAYERROR(jsonStateVars, GameData, "state", NULL);

  size_t totalSize = 0;
  size_t stateSize;

  cJSON *jsonStateVar;
  cJSON_ArrayForEach(jsonStateVar, jsonStateVars) {
    cJSON *jsonStateSize = cJSON_GetObjectItemCaseSensitive(jsonStateVar, "size");
    if (!cJSON_IsNumber(jsonStateSize)) {
      continue;
    }

    stateSize = cJSON_GetNumberValue(jsonStateSize);
    totalSize += stateSize;
#ifdef DEBUG_STATE
    printf("Found state variable of size %zu\n", stateSize);
#endif
  }

  // Round up size of last state to 8 bytes, this allows assigning the default value without
  // first checking the size. Without this the current code would write up to 7 bytes beyond
  // the end of the buffer.
#ifdef DEBUG_STATE
  printf("Total size is %zu, ", totalSize);
#endif
  if (stateSize != sizeof(uint64_t)) {
    totalSize += sizeof(uint64_t) - stateSize;
  }
#ifdef DEBUG_STATE
  printf("Rounded size is %zu\n", totalSize);
#endif

  unsigned char *gameState = malloc(totalSize);
  if (!gameState) {
    return NULL;
  }

  size_t currentOffset = 0;
  cJSON_ArrayForEach(jsonStateVar, jsonStateVars) {
    cJSON *jsonStateSize = cJSON_GetObjectItemCaseSensitive(jsonStateVar, "size");
    if (!cJSON_IsNumber(jsonStateSize)) {
      continue;
    }
    stateSize = cJSON_GetNumberValue(jsonStateSize);

    cJSON *jsonDefaultValue = cJSON_GetObjectItemCaseSensitive(jsonStateVar, "default");
    if (!cJSON_IsNumber(jsonDefaultValue)) {
      continue;
    }

    // Assume stateSize is 8, if not the next var's default value with override any higher set bits
    // and so any excess writing is irrelevant. Doing it this way avoids branching to check the var
    // size. The buffer is large enough so that even overestimating the size of the final variable
    // is safe.
    *((uint64_t *)gameState + currentOffset) = cJSON_GetNumberValue(jsonDefaultValue);
#ifdef DEBUG_STATE
    printf("Set state variable at offset %zu, remainder %zu of size %zu(8) to %" PRIx64 "\n",
           currentOffset, totalSize - currentOffset, stateSize,
           *((uint64_t *)gameState + currentOffset));
#endif
    currentOffset += stateSize;
  }

#ifdef DEBUG_STATE
  getchar();
#endif

  return gameState;
}

// TODO: Change error value to -1(would need nonstandard ssize_t)?
size_t GetGameStateOffset(enum Screen screenID, uint_fast8_t stateID) {
  if (!GameData) {
    return SIZE_MAX;
  }

  cJSON *jsonStateVars;
  JSON_GETJSONARRAYERROR(jsonStateVars, GameData, "state", SIZE_MAX);

  size_t currentOffset = 0;
  cJSON *jsonStateVar;

  cJSON_ArrayForEach(jsonStateVar, jsonStateVars) {
    cJSON *jsonStateSize = cJSON_GetObjectItemCaseSensitive(jsonStateVar, "size");
    if (!cJSON_IsNumber(jsonStateSize)) {
      goto loop;
    }

    cJSON *jsonCurScreenID = cJSON_GetObjectItemCaseSensitive(jsonStateVar, "screenID");
    if (!cJSON_IsNumber(jsonCurScreenID)) {
      goto loop;
    }

    enum Screen curScreenID = cJSON_GetNumberValue(jsonCurScreenID);
    if (screenID != curScreenID) {
      goto loop;
    }

    cJSON *jsonCurStateID = cJSON_GetObjectItemCaseSensitive(jsonStateVar, "stateID");
    if (!cJSON_IsNumber(jsonCurStateID)) {
      goto loop;
    }

    uint_fast8_t curStateID = cJSON_GetNumberValue(jsonCurStateID);
    if (stateID != curStateID) {
      goto loop;
    }

    return currentOffset;

loop:
    currentOffset += cJSON_GetNumberValue(jsonStateSize);
  }

  return SIZE_MAX;
}


// TODO: Change error value to 0? Would require making 0 screens not allowed which is reasonable
uint_fast16_t GetGameScreenCount(void) {
  if (!GameData) {
    return UINT_FAST16_MAX;
  }

  cJSON *jsonScreens;
  JSON_GETJSONARRAYERROR(jsonScreens, GameData, "screens", UINT_FAST16_MAX);

  return cJSON_GetArraySize(jsonScreens);
}

static cJSON *GetGameScreenJson(enum Screen screenID) {
  // cJSON_GetArrayItem uses int, likely fine as INT_MAX >= 2^15 - 1
  if (!GameData || screenID > INT_MAX) {
    return NULL;
  }

  cJSON *jsonScreens;
  JSON_GETJSONARRAYERROR(jsonScreens, GameData, "screens", NULL);

  cJSON *jsonScreen;
  JSON_GETJSONARRAYITEMOBJERROR(jsonScreen, jsonScreens, screenID, NULL);

  return jsonScreen;
}

bool GetGameScreen(enum Screen screenID, struct GameScreen *screen) {
  if (!screen) {
    return false;
  }

  cJSON *jsonScreen = GetGameScreenJson(screenID);
  if (!jsonScreen) {
    return false;
  }

  JSON_GETSTRINGVALUEERROR(screen->body, jsonScreen, "body", false);

  JSON_GETSTRINGVALUEERROR(screen->extraText, jsonScreen, "extraText", false);

  screen->customScreenCodeID = cJSON_GetOptNumberValue(jsonScreen, "customScreenCode", InvalidCustomScreenCode, invalidOptNumberVal);
  if (invalidOptNumberVal == screen->customScreenCodeID) {
    return false;
  }

  return true;
}


// TODO: Change error value to 0? Would require making 0 buttons not allowed which is reasonable
uint_fast8_t GetGameScreenButtonCount(enum Screen screenID) {
  cJSON *jsonScreen = GetGameScreenJson(screenID);
  if (!jsonScreen) {
    return UINT_FAST8_MAX;
  }

  cJSON *jsonButtons;
  JSON_GETJSONARRAYERROR(jsonButtons, jsonScreen, "buttons", UINT_FAST8_MAX);

  return cJSON_GetArraySize(jsonButtons);
}

bool GetGameScreenButton(enum Screen screenID, uint_fast8_t buttonID, struct GameScreenButton *button) {
  if (!button) {
    return false;
  }

  cJSON *jsonScreen = GetGameScreenJson(screenID);
  if (!jsonScreen) {
    return false;
  }

  cJSON *jsonButtons;
  JSON_GETJSONARRAYERROR(jsonButtons, jsonScreen, "buttons", false);

  cJSON *jsonButton;
  JSON_GETJSONARRAYITEMOBJERROR(jsonButton, jsonButtons, buttonID, false);

  JSON_GETSTRINGVALUEERROR(button->title, jsonButton, "title", false);

  button->outcome = cJSON_GetOptNumberValue(jsonButton, "outcome", InvalidInputOutcome, invalidOptNumberVal);

  button->newScreenID = cJSON_GetOptNumberValue(jsonButton, "newScreenID", InvalidScreen, invalidOptNumberVal);
  if (invalidOptNumberVal == button->newScreenID) {
    return false;
  }

  return true;
}


static bool GetGameRoomData(cJSON *jsonRoom, struct RoomInfo *room) {
  if (!jsonRoom || !room) {
    return false;
  }

  JSON_GETNUMBERVALUEERROR(room->type, jsonRoom, "type", false);

  // room->northRoomID = cJSON_GetOptNumberValue(jsonRoom, "north", InvalidRoomID, invalidOptNumberVal);
  // room->eastRoomID = cJSON_GetOptNumberValue(jsonRoom, "east", InvalidRoomID, invalidOptNumberVal);
  // room->southRoomID = cJSON_GetOptNumberValue(jsonRoom, "south", InvalidRoomID, invalidOptNumberVal);
  // room->westRoomID = cJSON_GetOptNumberValue(jsonRoom, "west", InvalidRoomID, invalidOptNumberVal);
  // if (invalidOptNumberVal == room->northRoomID || invalidOptNumberVal == room->eastRoomID
  // || invalidOptNumberVal == room->southRoomID || invalidOptNumberVal == room->westRoomID) {
  //   return false;
  // }

  return true;
}

static bool GetGameEquipmentItemData(cJSON *jsonItem, struct EquipmentInfo *item) {
  if (!jsonItem || !item) {
    return false;
  }

  JSON_GETSTRINGVALUEERROR(item->name, jsonItem, "name", false);

  JSON_GETNUMBERVALUEERROR(item->physAtkMod, jsonItem, "physAtkMod", false);
  JSON_GETNUMBERVALUEERROR(item->physDefMod, jsonItem, "physDefMod", false);
  JSON_GETNUMBERVALUEERROR(item->magAtkMod, jsonItem, "magAtkMod", false);
  JSON_GETNUMBERVALUEERROR(item->magDefMod, jsonItem, "magDefMod", false);

  return true;
}
