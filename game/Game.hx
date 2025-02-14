package game;

import game.Extensions;

enum Rooms {
  Empty;
  TestA;
  TestB(test: Int);
}

class Game implements Extension {
  final rooms = Rooms;
}
