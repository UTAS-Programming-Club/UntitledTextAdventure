package backend;

import backend.GameInfo;

@:nullSafety(Strict)
class ScreenAction {
  public final action: GameActions;
  public final title: UnicodeString;

  public function new(action: GameActions, title: UnicodeString) {
    this.action = action; 
    this.title = title;
  }
}

@:nullSafety(Strict)
class Screen {
  public final body: UnicodeString;
  public final actions: Array<ScreenAction>;

  public function new(body: UnicodeString, actions: Array<ScreenAction>) {
    this.body = body;
    this.actions = actions;
  }
}
