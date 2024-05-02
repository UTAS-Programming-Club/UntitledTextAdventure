//#define DEBUG_STATE

#include <assert.h>
#include <cJSON.h>
#include <b64.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "fileloading.h"
#include "winresources.h"
#include "parser.h"
#include "../shared/base64.h"

static void *Data = NULL;
static cJSON *GameData = NULL;

bool LoadGameData(char *path) {
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
    printf("ERROR: Unable to load %s, error in parsing at position %td.\n", path, cJSON_GetErrorPtr() - (char *)Data);
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


// Currently only integers are supported
// TODO: Support floating point
// TODO: Support loading data from "save", the plan is to use a password system so no actual saves per se
// Must be freed at the end of the program
unsigned char *InitGameState(void) {
  cJSON *stateVars = cJSON_GetObjectItemCaseSensitive(GameData, "state");
  if (!cJSON_IsArray(stateVars)) {
    return NULL;
  }

  size_t totalSize = 0;
  size_t stateSize;

  cJSON *stateVar;
  cJSON_ArrayForEach(stateVar, stateVars) {
    cJSON *jsonStateSize = cJSON_GetObjectItemCaseSensitive(stateVar, "size");
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
  cJSON_ArrayForEach(stateVar, stateVars) {
    cJSON *jsonStateSize = cJSON_GetObjectItemCaseSensitive(stateVar, "size");
    if (!cJSON_IsNumber(jsonStateSize)) {
      continue;
    }
    stateSize = cJSON_GetNumberValue(jsonStateSize);

    cJSON *defaultValue = cJSON_GetObjectItemCaseSensitive(stateVar, "default");
    if (!cJSON_IsNumber(defaultValue)) {
      continue;
    }

    // Assume stateSize is 8, if not the next var's default value with override any higher set bits
    // and so any excess writing is irrelevant. Doing it this way avoids branching to check the var
    // size. The buffer is large enough so that even overestimating the size of the final variable
    // is safe.
    *((uint64_t *)gameState + currentOffset) = cJSON_GetNumberValue(defaultValue);
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

size_t GetGameStateOffset(enum ScreenID screenID, uint8_t stateID) {
  cJSON *stateVars = cJSON_GetObjectItemCaseSensitive(GameData, "state");
  if (!cJSON_IsArray(stateVars)) {
    return SIZE_MAX;
  }

  size_t currentOffset = 0;
  cJSON *stateVar;

  cJSON_ArrayForEach(stateVar, stateVars) {
    cJSON *jsonStateSize = cJSON_GetObjectItemCaseSensitive(stateVar, "size");
    if (!cJSON_IsNumber(jsonStateSize)) {
      goto loop;
    }

    cJSON *jsonCurScreenID = cJSON_GetObjectItemCaseSensitive(stateVar, "screenID");
    if (!cJSON_IsNumber(jsonCurScreenID)) {
      goto loop;
    }

    enum ScreenID curScreenID = cJSON_GetNumberValue(jsonCurScreenID);
    if (screenID != curScreenID) {
      goto loop;
    }

    cJSON *jsonCurStateID = cJSON_GetObjectItemCaseSensitive(stateVar, "stateID");
    if (!cJSON_IsNumber(jsonCurStateID)) {
      goto loop;
    }

    uint8_t curStateID = cJSON_GetNumberValue(jsonCurStateID);
    if (stateID != curStateID){
      goto loop;
    }

    return currentOffset;

loop:
    currentOffset += cJSON_GetNumberValue(jsonStateSize);
  }

  return SIZE_MAX;
}


// TODO: Change error value to 0
uint16_t GetGameScreenCount(void) {
  if (!GameData) {
    return UINT16_MAX;
  }

  cJSON *screens = cJSON_GetObjectItemCaseSensitive(GameData, "screens");
  if (!cJSON_IsArray(screens)) {
    return UINT16_MAX;
  }

  return cJSON_GetArraySize(screens);
}

static cJSON *GetGameScreenJson(enum ScreenID screenID) {
  if (!GameData) {
    return NULL;
  }

  // cJSON_GetArrayItem uses int, likely fine as INT_MAX >= 2^15
  if (screenID > INT_MAX) {
    return NULL;
  }

  cJSON *screens = cJSON_GetObjectItemCaseSensitive(GameData, "screens");
  // TODO: Check if cJSON_GetArrayItem does this check
  if (!cJSON_IsArray(screens)) {
    return NULL;
  }

  cJSON *screen = cJSON_GetArrayItem(screens, screenID);
  if (!cJSON_IsObject(screen)) {
    return NULL;
  }

  return screen;
}

// Must free screen->body and screen->extraText if this returns true
bool GetGameScreen(enum ScreenID screenID, struct GameScreen *screen) {
  if (!screen) {
    return false;
  }

  cJSON *jsonScreen = GetGameScreenJson(screenID);
  if (!jsonScreen) {
    return false;
  }

  cJSON *jsonBody = cJSON_GetObjectItemCaseSensitive(jsonScreen, "body");
  char *base64Body = cJSON_GetStringValue(jsonBody);
  if (!base64Body) {
    return false;
  }

  cJSON *jsonExtraText = cJSON_GetObjectItemCaseSensitive(jsonScreen, "extraText");
  char *base64ExtraText = cJSON_GetStringValue(jsonExtraText);
  if (!base64ExtraText) {
    return false;
  }

  screen->body = c32base64toc32(base64Body);
  if (!screen->body) {
    return false;
  }

  screen->extraText = c32base64toc32(base64ExtraText);
  if (!screen->extraText) {
    free(screen->body);
    return false;
  }

  return true;
}


// TODO: Change error value to 0
uint8_t GetGameScreenButtonCount(enum ScreenID screenID) {
  cJSON *screen = GetGameScreenJson(screenID);
  if (!screen) {
    return UINT8_MAX;
  }

  cJSON *buttons = cJSON_GetObjectItemCaseSensitive(screen, "buttons");
  if (!cJSON_IsArray(buttons)) {
    return UINT8_MAX;
  }

  return cJSON_GetArraySize(buttons);
}

// Must free by calling FreeGameScreenButton
bool GetGameScreenButton(enum ScreenID screenID, uint8_t buttonID, struct GameScreenButton *button) {
  if (!button) {
    return false;
  }

  cJSON *screen = GetGameScreenJson(screenID);
  if (!screen) {
    return false;
  }

  cJSON *buttons = cJSON_GetObjectItemCaseSensitive(screen, "buttons");
  if (!cJSON_IsArray(buttons)) {
    return false;
  }

  cJSON *jsonButton = cJSON_GetArrayItem(buttons, buttonID);
  if (!cJSON_IsObject(screen)) {
    return false;
  }

  cJSON *jsonTitle = cJSON_GetObjectItemCaseSensitive(jsonButton, "title");
  char *base64Title = cJSON_GetStringValue(jsonTitle);
  if (!base64Title) {
    return false;
  }

  cJSON *jsonOutcome = cJSON_GetObjectItemCaseSensitive(jsonButton, "outcome");
  if (!cJSON_IsNumber(jsonOutcome)) {
    return false;
  }
  button->outcome = cJSON_GetNumberValue(jsonOutcome);

  if (button->outcome == GotoScreen) {
    cJSON *jsonNewScreen = cJSON_GetObjectItemCaseSensitive(jsonButton, "newScreen");
    if (!cJSON_IsNumber(jsonNewScreen)) {
      return false;
    }
    button->newScreen = cJSON_GetNumberValue(jsonNewScreen);
  } else {
    button->newScreen = InvaidScreenID;
  }

  button->title = c32base64toc32(base64Title);
  return button->title != NULL;
}

// Safe even if GetGameScreenButton failed
void FreeGameScreenButton(struct GameScreenButton *button) {
  free(button->title);
  button->title = NULL;
}
