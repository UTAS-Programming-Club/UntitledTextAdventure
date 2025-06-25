package extensions.rooms;

import backend.Extension;
import extensions.rooms.Screens;

// TODO: Merge into CoreGame?
@:nullSafety(Strict)
final RoomsExt: Extension = {
  actions: [],
  outcomes: [],
  screens: [{type: GameRooms, constructor: GameRooms.new}],
  /*actionHandler: function(state: Game, action: GameAction): GameOutcome {
    final screen: Screen = state.getScreen();
    final roomScreenState: Null<GameRoomState> = state.tryGetScreenState();
    if (roomScreenState == null) {
      return Invalid;
    }

    switch (action) {
      case GoNorth:
        roomScreenState.changeRoom(state, roomScreenState.x, roomScreenState.y + 1);
      case GoEast:
        roomScreenState.changeRoom(state, roomScreenState.x - 1, roomScreenState.y);
      case GoSouth:
        roomScreenState.changeRoom(state, roomScreenState.x, roomScreenState.y - 1);
      case GoWest:
        roomScreenState.changeRoom(state, roomScreenState.x + 1, roomScreenState.y);
    }
  }*/
};
