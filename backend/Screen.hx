package backend;

import backend.GameState;
import backend.Helpers;
import haxe.ds.Either;

@:nullSafety(Strict)
abstract class Screen {
  private final updateState: GameState -> UnicodeString;

  public function new(updateState: OneOf<UnicodeString, GameState -> UnicodeString>) {
    this.updateState = switch(updateState) {
      case Left(bodyStr):
        function (_: GameState) return bodyStr;
      case Right(updateStateFunc):
        updateStateFunc;
    }
  }

  public function GetBody(state: GameState): UnicodeString {
    return this.updateState(state);
  }
}

enum ScreenActionType {
  GotoScreen(screen: Screen);
  GoNorth;
  GoEast;
  GoSouth;
  GoWest;
  QuitGame;
}

enum ScreenActionOutcome {
  GetNextOutput;
  QuitGame;
}

class ScreenAction {
  public final title: UnicodeString;
  public final type: ScreenActionType;
  public final isVisible: GameState -> Bool;

  public function new(title: UnicodeString, type: ScreenActionType, ?isVisible: GameState -> Bool) {
    this.title = title;
    this.type = type;
    this.isVisible = isVisible ?? AlwaysVisible;
  }

  public static function AlwaysVisible(state: GameState): Bool {
    return true;
  }
}

class ActionScreen extends Screen {
  private var actions(default, null): Null<Array<ScreenAction>>;

  public function new(updateState: OneOf<UnicodeString, GameState -> UnicodeString>,
                      ?actions: Array<ScreenAction>) {
    super(updateState);
    if (actions != null) {
      this.actions = actions;
    }
  }

  public function Init(actions: Array<ScreenAction>): Void {
    this.actions ??= actions;
  }

  public function GetActions(state: GameState): Array<ScreenAction> {
    return [for (action in actions) if (action.isVisible(state)) action];
  }
}
