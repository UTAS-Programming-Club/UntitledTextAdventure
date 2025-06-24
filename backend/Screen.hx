package backend;

import backend.Campaign;
import backend.Game;
import backend.GameInfo;
import backend.Outcome;
import backend.macros.Helpers;
import backend.coregame.Outcomes;
import haxe.ds.Either;

abstract class Screen {
  public function new() {
  }

  public abstract function getBody(game: Game): UnicodeString;
}

class ScreenAction {
  public final action: GameAction;
  public final title: UnicodeString;
  public final isVisible: (Game, ActionScreen) -> Bool;
  private final outcome: Game -> GameOutcome;

  public function new(action: GameAction, title: UnicodeString,
                      ?isVisible: (Game, ActionScreen) -> Bool,
                      ?outcome: Game -> GameOutcome) {
    this.action = action;
    this.title = title;
    this.isVisible = isVisible ?? AlwaysVisible;
    this.outcome = outcome ?? AlwaysInvalidOutcome;
  }

  static function AlwaysVisible(Game, ActionScreen): Bool return true;
  static function AlwaysInvalidOutcome(Game): GameOutcome return Invalid;

  public function handleAction(state: Game): GameOutcome {
    final outcome: GameOutcome = this.outcome(state);
    if (outcome == Invalid) {
#if debug
      throw 'Unhandled action ${Type.getClassName(action).split('.').pop()} on ${state.getScreen()}.';
#else
      throw 'Unhandled action $action on ${state.getScreen()}.';
#end
    }

    return outcome;
  }
}

abstract class ActionScreen extends Screen {
  private abstract function getAllActions(): Array<ScreenAction>;

  public function GetActions(state: Game): Array<ScreenAction> {
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
