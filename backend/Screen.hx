package backend;

import backend.Game;
import backend.GameInfo;
import backend.macros.Helpers;
import haxe.ds.Either;

@:nullSafety(Strict)
abstract class Screen {
  private final updateState: Game -> UnicodeString;

  public function new(updateState: OneOf<UnicodeString, Game -> UnicodeString>) {
    this.updateState = switch(updateState) {
      case Left(bodyStr):
        function (_: Game) return bodyStr;
      case Right(updateStateFunc):
        updateStateFunc;
    }
  }

  public function GetBody(state: Game): UnicodeString {
    return this.updateState(state);
  }
}

@:nullSafety(Strict)
class ScreenAction {
  public final action: GameAction;
  public final title: UnicodeString;
  // public final isVisible: (GameState, Room, Bool) -> Bool;
  public final getOutcome: (Game) -> GameOutcome;

  public function new(action: GameAction, title: UnicodeString,
                      /*?isVisible: (GameState, Room, Bool) -> Bool,*/
                      getOutcome: (Game) -> GameOutcome) {
    this.action = action;
    this.title = title;
    // this.isVisible = isVisible ?? AlwaysVisible;
    this.getOutcome = getOutcome;
  }

  // static function AlwaysVisible(state: GameState, room: Room, roomState: Bool): Bool {
  //   return true;
  // }
}

@:nullSafety(Strict)
class ActionScreen extends Screen {
  private var actions(default, null): Null<Array<ScreenAction>>;

  public function new(updateState: OneOf<UnicodeString, Game -> UnicodeString>,
                      ?actions: Array<ScreenAction>) {
    super(updateState);
    if (actions != null) {
      this.actions = actions;
    }
  }

  public function Init(actions: Array<ScreenAction>): Void {
    this.actions ??= actions;
  }

  public function GetActions(state: Game): Array<ScreenAction> {
    // final room: Null<Room> = GlobalData.rooms[state.player.Y][state.player.X];
    // if (room == null) {
    //   throw new haxe.Exception("Room (" + state.player.X + ", " + state.player.Y + ") does not exist");
    // }

    // final roomStateRow: Vector<Bool> = state.roomState[state.player.Y];
    // final roomState: Bool = roomStateRow.length != 0 && roomStateRow[state.player.X];

    if (actions == null) {
      return [];
    }
    // return [for (action in actions) if (action.isVisible(state, room, roomState)) action];
    return actions;
  }
}
