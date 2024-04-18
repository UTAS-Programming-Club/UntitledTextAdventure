#include <assert.h>
#include <cJSON.h>
#include <b64.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <uchar.h>
#if _WIN32
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "parser.h"
#include "../shared/base64.h"

static void *Data = NULL;
static cJSON *GameData = NULL;

static bool LoadFile(char *path, size_t *size, void **data) {
#ifdef _WIN32
#error Memory mapping windows files is not currently supported
#else
  bool success = false;

  if (!data) {
    return false;
  }

  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    goto cleanup;
  }

  struct stat st;
  if (fstat(fd, &st)) {
    goto cleanup;
  }

  static_assert(sizeof(off_t) <= sizeof(size_t));
  *size = st.st_size;
  *data = malloc(*size);

  // If this file gets large then switch to mmap and possibly a streaming json library
  if (read(fd, *data, *size) == -1) {
    goto cleanup;
  }

  success = true;

cleanup:
  close(fd);
  return success;
#endif
}

static void UnloadFile(void *data) {
#ifdef _WIN32
#error Memory mapping windows files is not currently supported
#else
  free(data);
#endif
}

bool LoadGameData(char *path) {
  if (!path) {
    return false;
  }

  size_t size;
  if (!LoadFile(path, &size, &Data)) {
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

bool GetGameScreen(uint32_t screenID, struct GameScreen *screen) {
  if (!screen) {
    return NULL;
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

  button->title = c32base64toc32(base64Title);
  if (! button->title) {
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
    UnloadFile(Data);
    Data = NULL;
  }
}
