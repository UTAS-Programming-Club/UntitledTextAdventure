package backend;

import backend.GameState;
import backend.Helpers;
import haxe.ds.Either;

abstract class Screen {
  private final body: GameState -> UnicodeString;

  public function new(body: OneOf<UnicodeString, GameState -> UnicodeString>) {
    this.body = switch(body) {
      case Left(bodyStr):
        function (_: GameState) return bodyStr;
      case Right(bodyFunc):
        bodyFunc;
    }
  }

  public function GetBody(state: GameState): UnicodeString {
    return this.body(state);
  }
}

enum ScreenActionType {
  GotoScreen(screen: Screen);
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

  public function new(body: OneOf<UnicodeString, GameState -> UnicodeString>, ?actions: Array<ScreenAction>) {
    super(body);
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
