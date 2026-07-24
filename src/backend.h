#ifndef UTA_BACKEND_H
#define UTA_BACKEND_H

#include <stdbool.h>  // for bool
#include <stddef.h>   // for size_t, NULL
#include <stdint.h>   // for uint8_t

struct GameInfo {
  // Backend only, do not modify outside backend_setup, backend_register_extension and backend_cleanup
  uint8_t mapSizeX, mapSizeY;
  const struct Room **map; // const struct Room *map[maxSizeX * maxSizeY]

  // Frontend & Backend, do not modify from frontend
  const struct Screen *screen;
  uint8_t x, y;
  bool quit;
};

// Extension interface
#define NEW_ACTION(title, visibility_checker, trigger_handler) { title, visibility_checker, trigger_handler }
#define NEW_EXT_ACTION(title, visibility_checker, trigger_handler, ...) { NEW_ACTION(title, visibility_checker, trigger_handler), __VA_ARGS__ }
struct Action {
  // Frontend & Backend, do not modify
  const char *title;
  bool (*visibility_checker)(const struct GameInfo *info, const struct Action *action);

  // Backend only, do not modify
  bool (*trigger_handler)(struct GameInfo *info, const struct Action *action);
};

#define ARR_COUNT(ARR) sizeof(ARR) / sizeof(*ARR)
#define VA_ARGS_ARR(T, ...) (T[]){ __VA_ARGS__ }
#define VA_ARGS_ARR_COUNT(T, ...) ARR_COUNT(VA_ARGS_ARR(T, __VA_ARGS__))

#define NEW_SCREEN(body, ...) {                          \
  body, backend_default_screen_body_generator,           \
  VA_ARGS_ARR_COUNT(const struct Action *, __VA_ARGS__), \
  VA_ARGS_ARR(const struct Action *, __VA_ARGS__)        \
}
#define NEW_VAR_SCREEN(body_generator, ...) {            \
  NULL, body_generator,                                  \
  VA_ARGS_ARR_COUNT(const struct Action *, __VA_ARGS__), \
  VA_ARGS_ARR(const struct Action *, __VA_ARGS__)        \
}
struct Screen {
  // Backend only, do not modify
  const char *body;

  // Frontend & Backend, do not modify
  const char *(*body_generator)(const struct GameInfo *info);

  size_t actionCount;
  const struct Action *const *actions;
};

#define USE_ACTION(action) (const struct Action *)&action

# define NEW_ROOM(x, y, body) {x, y, body}
struct Room {
  // Backend only, do not change
  uint8_t x, y;
  const char *body;
};

bool backend_default_action_visibility_checker(const struct GameInfo *info, const struct Action *action);
bool backend_default_action_trigger_handler(struct GameInfo *info, const struct Action *action);

const char *backend_default_screen_body_generator(const struct GameInfo *info);

bool backend_register_extension(struct GameInfo *info, size_t roomCount, const struct Room *const rooms[static roomCount]);

const struct Room *backend_get_map_room(const struct GameInfo *info, uint8_t x, uint8_t y);

// Frontend interface
bool backend_setup(struct GameInfo *info);
const char *backend_output(const struct GameInfo *info);
bool backend_input(struct GameInfo *info, uint8_t actionId);
void backend_cleanup(struct GameInfo *info);

#endif // UTA_BACKEND_H
