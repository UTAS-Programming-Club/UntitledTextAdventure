#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_IMPLEMENTATION
#include <arena.h>

#include "game.h"
#include "screens.h"
#include "../shared/parser.h"
#include "../shared/strings.h"

bool CreateScreen(struct GameOutput *output) {
  if (!output) {
    return false;
  }

  struct GameScreen screen = {0};
  if (!GetGameScreen(output->screenID, &screen)) {
    return false;
  }
  output->body = screen.body;
  output->bodyArena = false;
  free(screen.extraText);
  output->customScreenID = screen.customScreenID;

  uint8_t buttonCount = GetGameScreenButtonCount(output->screenID);
  if (buttonCount == UINT8_MAX) {
    return false;
  }
  output->inputCount = buttonCount;

  output->inputs = arena_alloc(&output->arena, output->inputCount * sizeof *output->inputs);
  output->inputsArrayArena = true;
  if (!output->inputs) {
    return false;
  }

  for (uint8_t i = 0; i < buttonCount; ++i) {
    struct GameScreenButton button;
    if (!GetGameScreenButton(output->screenID, i, &button)) {
      return false;
    }
    output->inputs[i].title = button.title;
    output->inputs[i].titleArena = false;
  }

  return true;
}

// TODO: Remove once everything is using the arena
void FreeScreen(struct GameOutput *output) {
  if (!output->bodyArena) {
    free(output->body);
    output->body = NULL;
  }
  for (uint8_t i = 0; i < output->inputCount; ++i) {
    // This only works because FreeGameScreenButton only touches the string at the beginning of
    // GameScreenButton which matches the layout of GameInput
    // TODO: Use GameScreenButton everywhere?
    if (!output->inputs[i].titleArena) {
      FreeGameScreenButton((struct GameScreenButton *)(output->inputs + i));
    }
  }
  if (!output->inputsArrayArena) {
    free(output->inputs);
    output->inputs = NULL;
  }
}

// TODO: Remove this
bool HandleScreenInput(enum ScreenID screenID, uint8_t inputID, struct GameScreenButton *button) {
  return GetGameScreenButton(screenID, inputID, button);
}
