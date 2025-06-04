package backend.coregame;

import backend.GameInfo;

enum CoreGameAction {
  StartGame;
  // TODO: Replace with goto last screen
  GotoGameScreen;
  GotoScreen(screen: GameScreen);
  QuitGame;
}
