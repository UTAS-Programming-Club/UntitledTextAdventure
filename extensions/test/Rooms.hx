package extensions.test;

import backend.Game;
import backend.GameInfo;
import backend.Room;

final TestRoom: GameRoom = new Test_Test();
class Test_Test extends Room {
  override function getRoomBody(Game): UnicodeString return '\n\nThis is a test room!';
}
