package backend;

import backend.Extension;
import backend.GameInfo;

typedef Campaign = {
  final mainMenu: UnicodeString;

  final extensions: Array<Extension>;

  final initialScreen: GameScreen;
  final gameScreen: GameScreen;
}
