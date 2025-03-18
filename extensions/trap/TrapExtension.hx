package extensions.trap;

import backend.Extension;
import backend.Game;
import backend.GameInfo;
import extensions.rooms.Screens;
import extensions.trap.Rooms;

@:nullSafety(Strict)
final TrapExt: Extension = {
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
    switch (action) {
      case DodgeTrap:
        // TODO: Move to seperate function(s) to reuse
        final screen = state.getScreen();
        final roomScreen = cast(screen, GameRoomScreen);
        final x: UInt = roomScreen.x;
        final y: UInt = roomScreen.y;
        final room: GameRoom = state.campaign.rooms[x][y];

        if (room != Trap) {
          return Invalid;
        }

        final gameScreen: GameRoomScreen = cast screen;
        final roomState: TrapRoom = gameScreen.getRoomState(state, x, y);

        roomState.activatedTrap = true;
        return GetNextOutput;
      default:
        return Invalid;
    }
  }
};
