package game;

enum Rooms {
  Empty;
  TestA;
  TestB(test: Int);
}

class Game {
  static final rooms = Rooms;
}
