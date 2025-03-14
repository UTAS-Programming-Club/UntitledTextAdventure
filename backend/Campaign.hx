package backend;

import backend.GameInfo;

typedef Campaign = {
  final mainMenu: UnicodeString;

  final initialScreen: GameScreen;
  final gameScreen: GameScreen;
}
