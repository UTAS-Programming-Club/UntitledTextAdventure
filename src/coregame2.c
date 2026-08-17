#include <stdbool.h>       // for true, bool, false
#include <stddef.h>        // for NULL
#include <stdint.h>        // for uint8_t

#include "backend.h"       // GameInfo, backend_get_map_room, Action (ptr only), Room
#include "coregame2.h"
#include "gen/coregame.h"  // for Core_ScreenDirection, Core_GoDirectionAction

const uint8_t MapSizeX = 3;
const uint8_t MapSizeY = 2;
