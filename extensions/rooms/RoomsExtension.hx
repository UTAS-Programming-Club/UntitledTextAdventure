package extensions.rooms;

import backend.Extension;
import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.rooms.Screens;

// TODO: Merge into CoreGame?
@:nullSafety(Strict)
final RoomsExt: Extension = {
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
    final screen: Screen = state.getScreen();
    final roomScreen: GameRoomScreen = cast(screen, GameRoomScreen);

    switch (action) {
      case GoNorth:
        roomScreen.changeRoom(state, roomScreen.x, roomScreen.y + 1);
      case GoEast:
        roomScreen.changeRoom(state, roomScreen.x - 1, roomScreen.y);
      case GoSouth:
        roomScreen.changeRoom(state, roomScreen.x, roomScreen.y - 1);
      case GoWest:
        roomScreen.changeRoom(state, roomScreen.x + 1, roomScreen.y);
      default:
        return Invalid;
    }

    return GetNextOutput;
  }
};
