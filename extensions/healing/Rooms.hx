package extensions.healing;

import backend.Action;
import backend.Game;
import backend.Room;
import extensions.healing.Actions;

class HealingRoom extends Room {
  override function getRoomActions(): Array<Action> return [
    new HealPlayer('Heal'),
  ];

  override function getRoomBody(Game): UnicodeString return '\n\nThis is a healing fountain room.';
}
