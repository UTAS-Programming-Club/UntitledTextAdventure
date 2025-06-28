package backend;

import backend.Action;
import backend.Campaign;
import backend.Game;
import haxe.Constraints;

abstract class Screen {
  public function new() {
  }

  public function hasState(): Bool return false;
  public function createState(campaign: Campaign): ScreenState throw 'Screen has no state';

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

@:generic
abstract class StatefulActionScreen<T : ScreenState & Constructible<Campaign -> Void>> extends ActionScreen {
  override function hasState(): Bool return true;
  override function createState(campaign: Campaign): T return new T(campaign);

  abstract function getStatefulBody(state: Game, screenState: T): UnicodeString;

  function getBody(state: Game): UnicodeString return getStatefulBody(state, state.getScreenState());
}

abstract class ScreenState {
  public function new() {
  }
}
