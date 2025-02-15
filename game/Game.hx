package game;

import game.Extensions;

enum Rooms {
  Empty;
  TestA;
  TestB(test: Int);
}

enum Actions {
  GoNorth;
  GoEast;
  GoSouth;
  GoWest;
}

private final Game = new Extension(Rooms, Actions);
