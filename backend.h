#ifndef UTA_BACKEND_H
#define UTA_BACKEND_H

#include <stdbool.h>  // for bool
#include <stddef.h>   // for size_t
#include <stdint.h>   // for uint8_t

struct GameInfo {
  // Backend only, do not change outside backend_setup, backend_register_extension and backend_cleanup
  uint8_t mapSizeX, mapSizeY;
  const struct Screen **map; // const struct Screen *map[maxSizeX * maxSizeY]

  // Frontend & Backend, do not change from frontend
  const struct Screen *screen;
  uint8_t x, y;
  bool quit;
};

// Extension interface
#define NEW_ACTION(title, visibility_checker, trigger_handler) { title, visibility_checker, trigger_handler }
#define NEW_EXT_ACTION(title, visibility_checker, trigger_handler, ...) { NEW_ACTION(title, visibility_checker, trigger_handler), __VA_ARGS__ }
struct Action {
  // Frontend & Backend, do not change
  const char *title;
  bool (*visibility_checker)(const struct GameInfo *info, const struct Action *action);

  // Backend only, do not change
  bool (*trigger_handler)(struct GameInfo *info, const struct Action *action);
};

#define ARR_COUNT(ARR) sizeof(ARR) / sizeof(*ARR)
#define VA_ARGS_ARR(T, ...) (T[]){ __VA_ARGS__ }
#define VA_ARGS_ARR_COUNT(T, ...) ARR_COUNT(VA_ARGS_ARR(T, __VA_ARGS__))

#define NEW_SCREEN(x, y, body, ...) {                    \
  x, y, body,                                            \
  VA_ARGS_ARR_COUNT(const struct Action *, __VA_ARGS__), \
  VA_ARGS_ARR(const struct Action *, __VA_ARGS__)        \
}
struct Screen {
  uint8_t x, y;

  const char *body;

  size_t actionCount;
  const struct Action *const *actions;
};

#define USE_ACTION(action) (const struct Action *)&action

bool backend_default_action_visibility_checker(const struct GameInfo *info, const struct Action *action);
bool backend_default_action_trigger_handler(struct GameInfo *info, const struct Action *action);

bool backend_register_extension(struct GameInfo *info, size_t screenCount, const struct Screen *const screens[static screenCount]);

const struct Screen *backend_get_map_screen(const struct GameInfo *, uint8_t x, uint8_t y);

// Frontend interface
bool backend_setup(struct GameInfo *info);
const char *backend_output(const struct GameInfo *info);
bool backend_input(struct GameInfo *info, uint8_t actionId);
void backend_cleanup(struct GameInfo *info);

#endif // UTA_BACKEND_H
