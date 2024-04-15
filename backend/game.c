#include <stddef.h>

#include "game.h"

void GetCurrentGameOutput(struct GameOutput *output) {
  output->stateID = 0;
  output->body = U"Untitled text adventure game\n"
                U"----------------------------\n"
                U"By the UTAS Programming Club\n"
                U"\n"
                U"Currently unimplemented :(";
  output->inputCount = 0;
  output->inputs = NULL;
}

void CleanupGame(void) {
  FreeAll();
}
