package backend.coregame;

import backend.GameInfo;

enum Action {
  StartGame;
  GotoScreen(screen: GameScreen);
  GotoPreviousScreen;
  QuitGame;
}
