package backend;

import backend.Action;
import backend.Campaign;
import backend.Game;
import backend.GameInfo;
import backend.macros.Helpers;
import backend.Outcome;
import haxe.ds.Either;

abstract class Screen {
  public function new() {
  }

  public abstract function getBody(state: Game): UnicodeString;
}

abstract class ActionScreen extends Screen {
  private abstract function getAllActions(): Array<Action>;

  public function GetActions(state: Game): Array<Action> {
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

// TODO: Find a way to use a type parameter instead of a constructor parameter
/*abstract class StatefulActionScreen extends ActionScreen {
  public final stateConstructor: Campaign -> ScreenState;

  // TODO: Change Screen to ActionScreen if not StatefulActionScreen
  public function new(stateConstructor: Campaign -> ScreenState,
                      actions: Array<ScreenAction>) {
    super(actions);
    this.stateConstructor = stateConstructor;
  }
}*/

class ScreenState {
}
