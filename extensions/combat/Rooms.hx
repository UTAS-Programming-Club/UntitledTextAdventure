package extensions.combat;

import backend.Game;
import backend.GameInfo;
import backend.Room;

class CombatRoom extends Room {
  override function getRoomBody(Game): UnicodeString return '\n\nThis is the combat room.';
}
