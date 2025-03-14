package backend;

import backend.GameInfo;

typedef Campaign = {
  final name: UnicodeString;
  final author: UnicodeString;

  final initialScreen: GameScreen;
}
