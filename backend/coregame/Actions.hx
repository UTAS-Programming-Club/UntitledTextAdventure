package backend.coregame;

import backend.GameEnums;

enum CoreGameAction {
  StartGame;
  GotoScreen(screen: GameScreen);
  GotoPreviousScreen;
  QuitGame;
}
