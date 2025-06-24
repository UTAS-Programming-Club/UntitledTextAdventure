package extensions.rooms;

import backend.BaseGame;
import backend.Extension;
import backend.GameEnums;
// import backend.Screen;
import extensions.rooms.Actions;
import extensions.rooms.Rooms;
import extensions.rooms.ScreenInfo;
import extensions.rooms.Screens;

// TODO: Merge into CoreGame?
@:nullSafety(Strict)
final RoomsExt: Extension = {
  actions: [Actions],
  outcomes: [],
  rooms: [Room],
  screens: [RoomsScreen],
  // equipmentObjs: [],
  // roomObjs: [],
  screenObjs: [],
  actionHandler: function(state: BaseGame, action: GameAction): GameOutcome {
    /*final screen: Screen = state.getScreen();
    final roomScreenState: Null<GameRoomState> = state.tryGetScreenState();
    if (roomScreenState == null) {
      return Invalid;
    }*/
    trace(Type.getEnumConstructs(cast GameOutcome));
    Sys.getChar(false);

    switch (action) {
      /*case GoNorth:
        roomScreenState.changeRoom(state, roomScreenState.x, roomScreenState.y + 1);
      case GoEast:
        roomScreenState.changeRoom(state, roomScreenState.x - 1, roomScreenState.y);
      case GoSouth:
       roomScreenState.changeRoom(state, roomScreenState.x, roomScreenState.y - 1);
      case GoWest:
        roomScreenState.changeRoom(state, roomScreenState.x + 1, roomScreenState.y);*/
      default:
        return GameOutcome.Invalid;
    }


    // trace(Type.getEnumConstructs(GameAction));
    // trace(Type.getEnumConstructs(GameEquipment));
    // trace(Type.getEnumConstructs(GameRoom));
    // trace(Type.getEnumConstructs(GameScreen));
    // return GameOutcome.GetNextOutput;
  }
};
