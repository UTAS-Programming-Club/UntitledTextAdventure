#ifndef UTA_COREGAME2_H
#define UTA_COREGAME2_H

#include <stdbool.h>  // for bool
#include <stdint.h>   // for uint8_t

#include "backend.h"  // for Action, GameInfo (ptr only)
#include "gen/coregame.h"

extern const uint8_t MapSizeX;
extern const uint8_t MapSizeY;

const char *core_room_screen_body_generator(const struct GameInfo *const info);

#endif // UTA_COREGAME2_H
