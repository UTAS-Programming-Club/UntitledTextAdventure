package backend;

import backend.BaseGame;
import backend.Campaign;
import backend.GameEnums;
import backend.macros.Helpers;
import haxe.ds.Either;

abstract class Screen {
  private final updateState: (BaseGame, Screen) -> UnicodeString;

  public function new(updateState: OneOf<UnicodeString, (BaseGame, Screen) -> UnicodeString>) {
    this.updateState = switch(updateState) {
      case Left(bodyStr):
        function (BaseGame, Screen) return bodyStr;
      case Right(updateStateFunc):
        updateStateFunc;
    }
  }

  public function GetBody(state: BaseGame): UnicodeString {
    return this.updateState(state, this);
  }
}

class ScreenAction {
  public final action: GameAction;
  public final title: UnicodeString;
  // TODO: Add visibilityHandler in Extension?
  public final isVisible: (BaseGame, ActionScreen) -> Bool;
  // TODO: Is actionHandler in Extension enough? Remove this?
  private final outcome: BaseGame -> GameOutcome;

  public function new(action: GameAction, title: UnicodeString,
                      ?isVisible: (BaseGame, ActionScreen) -> Bool,
                      ?outcome: BaseGame -> GameOutcome) {
    this.action = action;
    this.title = title;
    this.isVisible = isVisible ?? AlwaysVisible;
    this.outcome = outcome ?? AlwaysInvalidOutcome;
  }

  static function AlwaysVisible(BaseGame, ActionScreen): Bool return true;
  static function AlwaysInvalidOutcome(BaseGame): GameOutcome return Invalid;

  public function handleAction(state: BaseGame): GameOutcome {
    var outcome: GameOutcome;
    for (ext in state.campaign.extensions) {
      if (ext.actionHandler == null) {
        continue;
      }

      final outcome: GameOutcome = ext.actionHandler(state, action);
      if (outcome != Invalid) {
        return outcome;
      }
    }

    outcome = this.outcome(state);
    if (outcome == Invalid) {
      throw 'Unhandled action outcome $action on ${state.getScreen()}.';
    }

    return outcome;
  }
}

class ActionScreen extends Screen {
  private final actions: Array<ScreenAction>;

  public function new(updateState: OneOf<UnicodeString, (BaseGame, Screen) -> UnicodeString>,
                      actions: Array<ScreenAction>) {
    super(updateState);
    this.actions = actions;
  }

  public function GetActions(state: BaseGame): Array<ScreenAction> {
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
                      updateState: OneOf<UnicodeString, (BaseGame, Screen) -> UnicodeString>,
                      actions: Array<ScreenAction>) {
    super(updateState, actions);
    this.stateConstructor = stateConstructor;
  }
}

class ScreenState {
}
