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
        roomScreen.y++;
        return GetNextOutput;
      case GoEast:
        roomScreen.x--;
        return GetNextOutput;
      case GoSouth:
        roomScreen.y--;
        return GetNextOutput;
      case GoWest:
        roomScreen.x++;
        return GetNextOutput;
      default:
        return Invalid;
    }
  }
};
