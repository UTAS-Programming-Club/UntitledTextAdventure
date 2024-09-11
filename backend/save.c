#include <arena.h>   // Arena, arena_alloc
#include <assert.h>  // static_assert
#include <math.h>    // pow
#include <stdbool.h> // bool, false, true
#include <stdint.h>  // uint8_t, uint16_t, uint32_t, uint_fast8_t, uint_fast32_t, UINT_FAST8_MAX
#include <string.h>  // memcpy, NULL, size_t, strlen
#include <types.h>   // EquipmentIDSave, EquipmentID, PlayerStatSave, RoomCoordSave
#include <zstd.h>    // ZSTD_compress, ZSTD_compressBound, ZSTD_CONTENTSIZE_ERROR, ZSTD_CONTENTSIZE_UNKNOWN, ZSTD_decompress, ZSTD_findFrameCompressedSize, ZSTD_getFrameContentSize, ZSTD_isError

#include "entities.h"  // RefreshStats
#include "equipment.h" // GetEquippedItemID, SetEquippedItem
#include "game.h"      // EquippedItemsSlots, struct GameInfo, struct GameState
#include "save.h"

#define BASE 85
#define STARTING_CHAR '!'
#define STARTING_CHAR_STR "!"

static_assert(sizeof(unsigned char) == sizeof(uint8_t), "Need 8 bit bytes.");
static_assert(sizeof(size_t) <= sizeof(unsigned long long), "Need to store size_t in unsigned long long.");

// 65535 is reserved
static const uint16_t PasswordVersion = 0;

struct __attribute__((packed, scalar_storage_order("little-endian"))) SaveData {
  uint16_t version;
  RoomCoordSave x;
  RoomCoordSave y;
  EntityStatSave health;
  EntityStatSave stamina;
  uint8_t unlockedItems[(EquipmentCount + 7) / 8];
  // TODO: EquipmentIDSave is [0, 63], switch to 6 bits items? ceil(6 * 7 / 8.) == 6, might not be worth it
  EquipmentIDSave equippedItems[EquipmentTypeCount];
};


static inline size_t min(size_t a, size_t b) {
  return a < b ? a : b;
}

// Assumes ascii
// TODO: Remove ` and \ to allow storing in discord
static inline char GetChar(uint_fast8_t val) {
  if (BASE < val) {
    return ' ';
  }

  return STARTING_CHAR + val;
}

// Assumes ascii
static inline uint_fast8_t GetVal(char c) {
  if (STARTING_CHAR > c || BASE + STARTING_CHAR <= c) {
    return UINT_FAST8_MAX;
  }

  return c - STARTING_CHAR;
}


// Encode state to ascii string using base 85 with 4 bytes to 5 chars
static const char *EncodeData(Arena *arena, const unsigned char *data, size_t dataSize, size_t *passwordSize) {
#ifdef _DEBUG
  if (!arena || !data || !passwordSize) {
    return NULL;
  }
#endif

  // Note that this overestimates if dataSize is not a multiple of 4
  // This is corrected by reversing the order of each group of chars
  // so that the password ends with ! which can then be stripped out
  // to get a password that is shorter by 0 to 4 chars.
  // Integer formula that gives the same result as 5*ceil(size/4.) for integer sizes
  *passwordSize = 5 * ((dataSize + 3) / 4);

  char *password = arena_alloc(arena, *passwordSize + 1);
  if (!password) {
    return NULL;
  }
  char *passwordPos = password;

  uint_fast32_t powers[] = {
    pow(BASE, 0),
    pow(BASE, 1),
    pow(BASE, 2),
    pow(BASE, 3),
    pow(BASE, 4)
  };

  for (size_t i = 0; i < dataSize; i += 4) {
    uint_fast32_t value = 0;
    size_t rem = dataSize - i % dataSize;
    size_t quot = min(rem, 4);
    memcpy(&value, data + i, quot);

    uint_fast8_t digit4 = value / powers[4];
    uint_fast32_t value4 = value - digit4 * powers[4];
#ifdef _DEBUG
    if (digit4 >= BASE) {
      return NULL;
    }
#endif

    uint_fast8_t digit3 = value4 / powers[3];
    uint_fast32_t value3 = value4 - digit3 * powers[3];
#ifdef _DEBUG
    if (digit3 >= BASE) {
      return NULL;
    }
#endif

    uint_fast8_t digit2 = value3 / powers[2];
    uint_fast32_t value2 = value3 - digit2 * powers[2];
#ifdef _DEBUG
    if (digit2 >= BASE) {
      return NULL;
    }
#endif

    uint_fast8_t digit1 = value2 / powers[1];
    uint_fast32_t value1 = value2 - digit1 * powers[1];
#ifdef _DEBUG
    if (digit1 >= BASE) {
      return NULL;
    }
#endif

    uint_fast8_t digit0 = value1 / powers[0];
#ifdef _DEBUG
    uint_fast32_t value0 = value1 - digit0 * powers[0];
    if (0 != value0) {
      return NULL;
    }
#endif

    *(passwordPos++) = GetChar(digit0);
    *(passwordPos++) = GetChar(digit1);
    *(passwordPos++) = GetChar(digit2);
    *(passwordPos++) = GetChar(digit3);
    *(passwordPos++) = GetChar(digit4);
  }

  for (int i = 0; i < 4 && passwordPos[-1] == STARTING_CHAR; ++i) {
    --passwordPos;
    --*passwordSize;
  }
  *passwordPos = '\0';

  return password;
}

static const void *DecodeData(Arena *arena, const char *password, size_t expectedSize, size_t *dataSize) {
#ifdef _DEBUG
  if (!arena || !password || !dataSize) {
    return NULL;
  }
#endif

  size_t passwordSize = strlen(password);
  // Integer formula that gives the same result as 4*ceil(size/5.) for integer sizes
  // This overshots which is corrected by shrinking to match expectedSize at the end
  *dataSize = 4 * ((passwordSize + 4) / 5);

  uint32_t *decodedData = arena_alloc(arena, *dataSize);
  if (!decodedData) {
    return NULL;
  }
  // TODO: Only set the last 4*ceil(size/5.) - ceil((4*size)/5.) bytes?
  memset(decodedData, 0, *dataSize);

  uint_fast32_t powers[] = {
    pow(BASE, 0),
    pow(BASE, 1),
    pow(BASE, 2),
    pow(BASE, 3),
    pow(BASE, 4)
  };

  for (size_t i = 0; i < passwordSize; i += 5) {
    char data[5] = STARTING_CHAR_STR STARTING_CHAR_STR STARTING_CHAR_STR STARTING_CHAR_STR STARTING_CHAR_STR;
    uint8_t *pData = (uint8_t *)&data;
    size_t rem = passwordSize - i % passwordSize;
    size_t quot = min(rem, 5);
    memcpy(pData, password + i, quot);

    uint_fast8_t digit0 = GetVal(pData[0]);
    uint_fast8_t digit1 = GetVal(pData[1]);
    uint_fast8_t digit2 = GetVal(pData[2]);
    uint_fast8_t digit3 = GetVal(pData[3]);
    uint_fast8_t digit4 = GetVal(pData[4]);
#ifdef _DEBUG
    if (UINT_FAST8_MAX == digit0 || UINT_FAST8_MAX == digit1 || UINT_FAST8_MAX == digit2
        || UINT_FAST8_MAX == digit3 || UINT_FAST8_MAX == digit4) {
      return NULL;
    }
#endif

    decodedData[i / 5] = digit4 * powers[4] + digit3 * powers[3] + digit2 * powers[2] + digit1 * powers[1] + digit0 * powers[0];
  }

  while (*dataSize > expectedSize && !*((uint8_t *)decodedData + *dataSize - 1)) {
    --*dataSize;
  }

  return decodedData;
}


static const void *CompressData(Arena *arena, const void *data, size_t dataSize, size_t *compressedDataSize) {
#ifdef _DEBUG
  if (!arena || !data || !compressedDataSize) {
    return NULL;
  }
#endif

  size_t maxCompressedDataSize = ZSTD_compressBound(dataSize);
  if (ZSTD_isError(maxCompressedDataSize)) {
    return NULL;
  }

  void *compressedData = arena_alloc(arena, maxCompressedDataSize);
  if (!compressedData) {
    return NULL;
  }

  // TODO: Try other compression levels
  *compressedDataSize = ZSTD_compress(compressedData, maxCompressedDataSize, data, dataSize, 3);
  if (ZSTD_isError(*compressedDataSize)) {
    return NULL;
  }

  return compressedData;
}

static bool DecompressData(Arena *arena, const void *data, size_t dataSize, void **decompressedData, unsigned long long *decompressedDataSize) {
#ifdef _DEBUG
  if (!arena || !data || !decompressedData || !decompressedDataSize) {
    return NULL;
  }
#endif

  *decompressedDataSize = ZSTD_getFrameContentSize(data, dataSize);
  if (ZSTD_CONTENTSIZE_UNKNOWN == *decompressedDataSize) {
    return false;
  }

  if (ZSTD_CONTENTSIZE_ERROR == *decompressedDataSize) {
    *decompressedData = NULL;
    return true;
  }

  size_t compressedDataSize = ZSTD_findFrameCompressedSize(data, dataSize);
  if (ZSTD_isError(compressedDataSize)) {
    return false;
  }

  *decompressedData = arena_alloc(arena, *decompressedDataSize);
  if (!*decompressedData) {
    return false;
  }

  size_t res = ZSTD_decompress(*decompressedData, *decompressedDataSize, data, compressedDataSize);
  if (ZSTD_isError(res)) {
    return false;
  }

  return true;
}


static const char *CompressAndEncodeData(Arena *arena, const void *data, size_t dataSize) {
#if _DEBUG
  if (!arena || !data) {
    return NULL;
  }
#endif

  // TODO: Determine encodedDataSize without calling EncodeData, only need the size unless compressedDataSize is larger
  size_t encodedDataSize;
  const char *encodedData = EncodeData(arena, data, dataSize, &encodedDataSize);
  if (!encodedData) {
    return NULL;
  }

  size_t compressedDataSize;
  const void *compressedData = CompressData(arena, data, dataSize, &compressedDataSize);
  if (!compressedData) {
    return NULL;
  }

  size_t encodedCompressedDataSize;
  const char *encodedCompressedData = EncodeData(arena, compressedData, compressedDataSize, &encodedCompressedDataSize);
  if (!encodedCompressedData) {
    return NULL;
  }

  const char *password;
  if (encodedCompressedDataSize < encodedDataSize) {
    password = encodedCompressedData;
  } else {
    password = encodedData;
  }

  return password;
}

static const void *DecodeAndDecompressData(Arena *arena, const char *password, size_t expectedSize, unsigned long long *dataSize) {
#ifdef _DEBUG
  if (!arena || !password || !dataSize) {
    return NULL;
  }
#endif

  size_t decodedPasswordSize;
  const void *decodedPassword = DecodeData(arena, password, expectedSize, &decodedPasswordSize);
  if (!decodedPassword) {
    return NULL;
  }

  unsigned long long decompressedDataSize;
  void *decompressedData;
  bool decompressionResult = DecompressData(arena, decodedPassword, decodedPasswordSize, &decompressedData, &decompressedDataSize);
  if (!decompressionResult) {
    return NULL;
  }

  if (!decompressedData) {
    *dataSize = decodedPasswordSize;
    return decodedPassword;
  }

  *dataSize = decompressedDataSize;
  return decompressedData;
}


// TODO: Test zstd compression
const char *SaveState(const struct GameInfo *info, struct GameState *state) {
  if (!state || !state->startedGame) {
    return NULL;
  }

  const struct RoomInfo *currentRoom = GetCurrentGameRoom(info, state);
  if (currentRoom->type == InvalidRoomType) {
    return InvalidInputOutcome;
  }

  struct SaveData *data;
  size_t dataSize = sizeof *data + state->stateDataSize;
  data = arena_alloc(&state->arena, dataSize);
  if (!data) {
    return NULL;
  }
  unsigned char *pData = (unsigned char *)data;

  data->version = PasswordVersion;
  // TODO: Switch to roomID
  data->x       = currentRoom->x;
  data->y       = currentRoom->y;
  data->health  = state->playerInfo.health;
  data->stamina = state->playerInfo.stamina;

  memset(data->unlockedItems, 0, sizeof data->unlockedItems);
  // TODO: Unroll to multiples of 8
  for (EquipmentID i = 0; i < EquipmentCount; ++i) {
    bool unlocked;
    if (!CheckItemUnlocked(&state->playerInfo, i, &unlocked)) {
      return NULL;
    }

    uint_fast8_t arrIdx = i / 8;
    uint_fast8_t arrOff = i % 8;
    data->unlockedItems[arrIdx] |= unlocked << arrOff;
  }

  for (enum EquipmentType i = 0; i < EquipmentTypeCount; ++i) {
    // Maps [0, 7*9) = [0, 62] to [1, 63] so 0 can be the invalid value
    EquipmentID id = GetEquippedItemID(&state->playerInfo, i);
    data->equippedItems[i] = InvalidEquipmentID != id ? id + 1 : InvalidEquipmentIDSave;
  }

  memcpy(pData + sizeof *data, state->stateData, state->stateDataSize);

  return CompressAndEncodeData(&state->arena, pData, dataSize);
}

bool LoadState(const struct GameInfo *info, struct GameState *state, const char *password) {
  if (!info || !info->initialised || !state || state->startedGame || !password) {
    return false;
  }

  struct SaveData *data;
  size_t expectedSize = sizeof *data + state->stateDataSize;
  unsigned long long dataSize;
  data = (struct SaveData *)DecodeAndDecompressData(&state->arena, password, expectedSize, &dataSize);
  if (!data || expectedSize != dataSize) {
    return false;
  }

  if (PasswordVersion != data->version) {
    return false;
  }

  state->playerInfo.health = data->health;
  state->playerInfo.stamina = data->stamina;

  // TODO: Unroll to multiples of 8?
  for (EquipmentID i = 0; i < EquipmentCount; ++i) {
    uint_fast8_t arrIdx = i / 8;
    uint_fast8_t arrOff = i % 8;

    if (!(data->unlockedItems[arrIdx] & 1 << arrOff)) {
      continue;
    }
    if (!UnlockItem(&state->playerInfo, i)) {
      return NULL;
    }
  }

  for (enum EquipmentType i = 0; i < EquipmentTypeCount; ++i) {
    EquipmentIDSave idSave = data->equippedItems[i];
    EquipmentID id = idSave != InvalidEquipmentIDSave ? idSave - 1 : InvalidEquipmentID;

    if (!SetEquippedItem(&state->playerInfo, i, id)) {
      return false;
    }
  }
  if (!RefreshPlayerStats(info, state)) {
    return false;
  }

  memcpy(state->stateData, (uint8_t *)data + sizeof *data, state->stateDataSize);

  state->previousScreenID = InvalidScreen;
  state->previousRoomID = SIZE_MAX;
  state->roomID = GetGameRoomID(info, data->x, data->y);
  state->startedGame = true;
  state->combatInfo.inCombat = false;

  return true;
}

bool CreateNewState(const struct GameInfo *info, struct GameState *state) {
  if (!info || !info->initialised || !state) {
    return false;
  }

  memcpy(&state->playerInfo, &info->defaultPlayerInfo, sizeof info->defaultPlayerInfo);
  if (!RefreshPlayerStats(info, state)) {
    return false;
  }

  // TODO: Reset state, requires removing main menu state

  state->previousScreenID = InvalidScreen;
  state->previousRoomID = SIZE_MAX;
  state->roomID = GetGameRoomID(info, DefaultRoomCoordX, DefaultRoomCoordY);
  state->startedGame = true;
  state->combatInfo.inCombat = false;
  return true;
}
