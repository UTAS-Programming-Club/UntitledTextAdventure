package backend;

import backend.GameState;
import backend.Helpers;
import haxe.ds.Either;
import haxe.ds.Vector;

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
  DodgeTrap;
  QuitGame;
}

enum ScreenActionOutcome {
  GetNextOutput;
  QuitGame;
}

@:nullSafety(Strict)
class ScreenAction {
  public final title: UnicodeString;
  public final type: ScreenActionType;
  public final isVisible: (GameState, Room, Bool) -> Bool;

  public function new(title: UnicodeString, type: ScreenActionType,
                      ?isVisible: (GameState, Room, Bool) -> Bool) {
    this.title = title;
    this.type = type;
    this.isVisible = isVisible ?? AlwaysVisible;
  }

  public static function AlwaysVisible(state: GameState, room: Room, roomState: Bool): Bool {
    return true;
  }
}

@:nullSafety(Strict)
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
    final room: Null<Room> = GlobalData.rooms[state.player.Y][state.player.X];
    if (room == null) {
      #if picovision
      return [];
      #else
      throw new haxe.Exception("Room (" + state.player.X + ", " + state.player.Y + ") does not exist");
      #end
    }

    final roomStateRow: Vector<Bool> = state.roomState[state.player.Y];
    final roomState: Bool = roomStateRow.length != 0 && roomStateRow[state.player.X];

    if (actions == null) {
      return [];
    }
    return [for (action in actions) if (action.isVisible(state, room, roomState)) action];
  }
}
