package backend;

import backend.Campaign;
import backend.GameEnums;
import backend.macros.Helpers;
import haxe.ds.Either;

abstract class Screen {
  private final updateState: (Game, Screen) -> UnicodeString;

  public function new(updateState: OneOf<UnicodeString, (Game, Screen) -> UnicodeString>) {
    this.updateState = switch(updateState) {
      case Left(bodyStr):
        function (Game, Screen) return bodyStr;
      case Right(updateStateFunc):
        updateStateFunc;
    }
  }

  public function GetBody(state: Game): UnicodeString {
    return this.updateState(state, this);
  }
}

class ScreenAction {
  public final action: GameAction;
  public final title: UnicodeString;
  // TODO: Add visibilityHandler in Extension?
  public final isVisible: (Game, ActionScreen) -> Bool;
  // TODO: Is actionHandler in Extension enough? Remove this?
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
    var outcome: GameOutcome;
    /*for (ext in state.campaign.extensions) {
      final outcome: GameOutcome = ext.actionHandler(state, action);
      if (outcome != Invalid) {
        return outcome;
      }
    }*/

    outcome = this.outcome(state);
    if (outcome == Invalid) {
      throw 'Unhandled action outcome $action on ${state.getScreen()}.';
    }

    return outcome;
  }
}

class ActionScreen extends Screen {
  private final actions: Array<ScreenAction>;

  public function new(updateState: OneOf<UnicodeString, (Game, Screen) -> UnicodeString>,
                      actions: Array<ScreenAction>) {
    super(updateState);
    this.actions = actions;
  }

  public function GetActions(state: Game): Array<ScreenAction> {
    // final room: Null<Room> = GlobalData.rooms[state.player.Y][state.player.X];
    // if (room == null) {
    //   throw new haxe.Exception("Room (" + state.player.X + ", " + state.player.Y + ") does not exist");
    // }

    // final roomStateRow: Vector<Bool> = state.roomState[state.player.Y];
    // final roomState: Bool = roomStateRow.length != 0 && roomStateRow[state.player.X];

    // TODO: Use this
    // return [for (action in actions) if (action.isVisible(state, this)) action];
    return actions;
  }
}

// TODO: Find a way to use a type parameter instead of a constructor parameter
class StatefulActionScreen extends ActionScreen {
  public final stateConstructor: Campaign -> ScreenState;

  // TODO: Change Screen to ActionScreen if not StatefulActionScreen
  public function new(stateConstructor: Campaign -> ScreenState,
                      updateState: OneOf<UnicodeString, (Game, Screen) -> UnicodeString>,
                      actions: Array<ScreenAction>) {
    super(updateState, actions);
    this.stateConstructor = stateConstructor;
  }
}

class ScreenState {
}
