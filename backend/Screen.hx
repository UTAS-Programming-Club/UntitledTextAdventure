package backend;

import haxe.Constraints;

import backend.Action;
import backend.Game;

abstract class Screen {
  public function new() {
  }

  public function hasState(): Bool return false;
  public function createState(): ScreenState throw ': Screen has no state';

  public abstract function getBody(state: Game): UnicodeString;
}

abstract class ActionScreen extends Screen {
  private abstract function getAllActions(): Array<Action>;

  public function GetActions(): Array<Action> {
    // final room: Null<Room> = GlobalData.rooms[state.player.Y][state.player.X];
    // if (room == null) {
    //   throw new haxe.Exception("Room (" + state.player.X + ", " + state.player.Y + ") does not exist");
    // }

    // final roomStateRow: Vector<Bool> = state.roomState[state.player.Y];
    // final roomState: Bool = roomStateRow.length != 0 && roomStateRow[state.player.X];

    // TODO: Use this
    // return [for (action in actions) if (action.isVisible(state, this)) action];
    return getAllActions();
  }
}

// Keep in sync with StatefulRoom in Room.hx
@:generic
abstract class StatefulActionScreen<T : ScreenState & Constructible<Void -> Void>> extends ActionScreen {
  override function hasState(): Bool return true;
  override function createState(): T return new T();

  abstract function getStatefulBody(state: Game, screenState: T): UnicodeString;

  function getBody(state: Game): UnicodeString return getStatefulBody(state, state.getScreenState());
}

abstract class ScreenState {
  public function new() {
  }
}
