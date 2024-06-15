#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_IMPLEMENTATION
#include <arena.h>

#include "game.h"
#include "screens.h"
#include "../shared/parser.h"

bool CreateScreen(struct GameOutput *output) {
  if (!output) {
    return false;
  }

  struct GameScreen screen = {0};
  if (!GetGameScreen(output->screenID, &screen)) {
    return false;
  }
  output->body = screen.body;
  output->customScreenCodeID = screen.customScreenCodeID;

  uint_fast8_t buttonCount = GetGameScreenButtonCount(output->screenID);
  if (buttonCount == UINT_FAST8_MAX) {
    return false;
  }
  output->inputCount = buttonCount;

  output->inputs = arena_alloc(&output->arena, output->inputCount * sizeof *output->inputs);
  if (!output->inputs) {
    return false;
  }

  for (uint_fast8_t i = 0; i < buttonCount; ++i) {
    struct GameScreenButton button;
    if (!GetGameScreenButton(output->screenID, i, &button)) {
      return false;
    }
    output->inputs[i].title = button.title;
    output->inputs[i].visible = true;
    output->inputs[i].outcome = button.outcome;
  }

  return true;
}
