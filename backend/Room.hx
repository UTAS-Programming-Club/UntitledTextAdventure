package backend;

import haxe.Constraints;

import backend.Action;
import backend.Game;
import backend.Screen;

// TODO: Merge Rooms_GameRooms into this class
abstract class Room extends ActionScreen {
  public function hasRoomState(): Bool return false;
  public function createRoomState(): ScreenState throw ': Room has no state';

  function getBody(state: Game): UnicodeString return "";
  function getAllActions(): Array<Action> return [];
}

@:generic
abstract class StatefulRoom<T : ScreenState & Constructible<Void -> Void>> extends Room {
  override function hasRoomState(): Bool return true;
  override function createRoomState(): T return new T();
}
