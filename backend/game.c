#include <stddef.h>

#include "alloc.h"
#include "game.h"

// TODO: Parse game states from JSON
#define MAINMENU 0
  #define STARTBUTTON 0
  #define QUITBUTTON 1

void GetCurrentGameOutput(struct GameOutput *output) {
  output->stateID = 0;
  output->body = U"Untitled text adventure game\n"
                U"----------------------------\n"
                U"By the UTAS Programming Club\n"
                U"\n"
                U"Currently unimplemented :(";

  output->inputCount = 2;
  // TODO: Handle Allocate returning NULL
  output->inputs = Allocate(output->inputCount * sizeof *output->inputs);
  output->inputs[0].inputID = 0;
  output->inputs[0].title = U"Start Game";
  output->inputs[1].inputID = 1;
  output->inputs[1].title = U"Quit Game";
}

enum GameInputOutcome HandleGameInput(uint32_t stateID, uint32_t inputID) {
  switch (stateID) {
    case MAINMENU:
      switch(inputID) {
        case STARTBUTTON:
          return GetNextOutput;
        case QUITBUTTON:
          return QuitGame;
      }
    break;
  }

  return InvalidInput;
}

void CleanupGame(void) {
  FreeAll();
}
