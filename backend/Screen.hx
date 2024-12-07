package backend;

import backend.GameState;

abstract class Screen {
  public final body: UnicodeString;

  public function new(body: UnicodeString) {
    this.body = body;
  }
}

enum ScreenActionType {
  GotoScreen(screen: Screen);
}

class ScreenAction {
  public final title: String;
  public final type: ScreenActionType;
  public final isVisible: GameState -> Bool;

  public function new(title: String, type: ScreenActionType, isVisible: GameState -> Bool) {
    this.title = title;
    this.type = type;
    this.isVisible = isVisible;
  }
}

class ActionScreen extends Screen {
  final actions: Array<ScreenAction>;

  public function new(body: UnicodeString, actions: Array<ScreenAction>) {
    super(body);
    this.actions = actions;
  }

  public function GetActions(state: GameState): Array<ScreenAction> {
    return [for (action in actions) if (action.isVisible(state)) action];
  }
}
