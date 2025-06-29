package backend;

import backend.Action;
import backend.Game;
import backend.Screen;

// TODO: Merge Rooms_GameRooms into this class
abstract class Room extends ActionScreen {
  function getBody(state: Game): UnicodeString return "";
  function getAllActions(): Array<Action> return [];
}
