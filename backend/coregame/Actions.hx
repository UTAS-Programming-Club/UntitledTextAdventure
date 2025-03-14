package backend.coregame;

import backend.GameInfo;

enum CoreGameAction {
  StartGame;
  GotoScreen(screen: GameScreen);
  QuitGame;
}
