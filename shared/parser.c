#include <assert.h>
#include <cJSON.h>
#include <b64.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
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
static uint32_t MainMenuScreenID = INVALID_SCREEN_ID;

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
    return false;
  }
  
  return true;
}

uint32_t GetGameScreenCount(void) {
  if (!GameData) {
    return UINT32_MAX;
  }

  cJSON *screens = cJSON_GetObjectItemCaseSensitive(GameData, "screens");
  if (!cJSON_IsArray(screens)) {
    return UINT32_MAX;
  }

  return cJSON_GetArraySize(screens);
}

static cJSON *GetGameScreenJson(uint32_t screenID) {
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

uint32_t GetMainMenuScreenID(void) {
  if (MainMenuScreenID != INVALID_SCREEN_ID) {
    return MainMenuScreenID;
  }

  uint32_t screenCount = GetGameScreenCount();
  for (uint32_t i = 0; i < screenCount; ++i) {
    cJSON *screen = GetGameScreenJson(i);
    if (!screen) {
      return INVALID_SCREEN_ID;
    }

    cJSON *mainMenu = cJSON_GetObjectItemCaseSensitive(screen, "mainMenu");
    if (cJSON_IsBool(mainMenu)) {
      MainMenuScreenID = i;
      return MainMenuScreenID;
    }
  }

  return INVALID_SCREEN_ID;
}

bool GetGameScreen(uint32_t screenID, struct GameScreen *screen) {
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

uint8_t GetGameScreenButtonCount(uint32_t screenID) {
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

bool GetGameScreenButton(uint32_t screenID, uint8_t buttonID, struct GameScreenButton *button) {
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

  button->title = c32base64toc32(base64Title);
  if (!button->title) {
    return false;
  }

  cJSON *jsonOutcome = cJSON_GetObjectItemCaseSensitive(jsonButton, "outcome");
  if (!cJSON_IsNumber(jsonOutcome)) {
    free(button->title);
    return false;
  }
  button->outcome = cJSON_GetNumberValue(jsonOutcome);

  if (button->outcome == GotoScreen) {
    cJSON *jsonNewScreen = cJSON_GetObjectItemCaseSensitive(jsonButton, "newScreen");
    if (!cJSON_IsNumber(jsonNewScreen)) {
      free(button->title);
      return false;
    }
    button->newScreen = cJSON_GetNumberValue(jsonNewScreen);
  } else {
    button->newScreen = INVALID_SCREEN_ID;
  }

  return true;
}

void FreeGameScreenButton(struct GameScreenButton *button) {
  free(button->title);
  button->title = NULL;
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
