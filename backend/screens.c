#include <arena.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_IMPLEMENTATION
#include <arena.h>
#include <stdbool.h>

#include "game.h"
#include "parser.h"
#include "screens.h"

bool CreateScreen(struct GameState *state) {
  if (!state) {
    return false;
  }

  struct GameScreen screen = {0};
  if (!GetGameScreen(state->screenID, &screen)) {
    return false;
  }
  state->body = screen.body;
  state->customScreenCodeID = screen.customScreenCodeID;
  state->inputType = screen.inputType;

  if (state->inputType == ButtonScreenInputType) {
    uint_fast8_t buttonCount = GetGameScreenButtonCount(state->screenID);
    if (buttonCount == UINT_FAST8_MAX) {
      return false;
    }
    state->inputCount = buttonCount;

    state->inputs = arena_alloc(&state->arena, state->inputCount * sizeof *state->inputs);
    if (!state->inputs) {
      return false;
    }

    for (uint_fast8_t i = 0; i < buttonCount; ++i) {
      struct GameScreenButton button;
      if (!GetGameScreenButton(state->screenID, i, &button)) {
        return false;
      }
      state->inputs[i].title = button.title;
      state->inputs[i].visible = true;
      state->inputs[i].outcome = button.outcome;
    }
  } else if (state->inputType == TextScreenInputType) {
    state->previousScreenID = screen.previousScreenID;
    state->nextScreenID = screen.nextScreenID;
  } else {
    return false;
  }

  return true;
}
