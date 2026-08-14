#ifndef UTA_COREGAME2_H
#define UTA_COREGAME2_H

#include <stdbool.h>  // for bool
#include <stdint.h>   // for uint8_t

#include "backend.h"  // for Action, GameInfo (ptr only)
#include "gen/coregame.h"

extern const uint8_t MapSizeX;
extern const uint8_t MapSizeY;

bool core_go_direction_visibility_checker(const struct GameInfo *const info, const struct Action *const action);

bool core_go_direction_action_handler(struct GameInfo *info, const struct Action *action);
bool core_test_action_handler(struct GameInfo *const info, const struct Action *const action);
bool core_quit_game_action_handler(struct GameInfo *const info, const struct Action *const action);

const char *core_room_screen_body_generator(const struct GameInfo *const info);

#endif // UTA_COREGAME2_H
