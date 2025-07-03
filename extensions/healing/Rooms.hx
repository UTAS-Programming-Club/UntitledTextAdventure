package extensions.healing;

import backend.Action;
import backend.Game;
import backend.GameInfo;
import backend.Room;
import extensions.healing.Actions;

final HealingRoom: GameRoom = new Healing_Healing();
class Healing_Healing extends Room {
  override function getRoomActions(): Array<Action> return [
    new HealPlayer('Heal'),
  ];

  override function getRoomBody(Game): UnicodeString return '\n\nThis is a healing fountain room.';
}
