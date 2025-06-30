package extensions.trap;

import backend.Extension;
// import backend.Game;
// import backend.GameInfo;
// import backend.Screen;
// import extensions.rooms.Screens;
import extensions.trap.Rooms;

@:nullSafety(Strict)
final TrapExt: Extension = {
  module: 'extensions.trap',
  actions: [],
  outcomes: [],
  rooms: [
    TrapRoom,
  ],
  screens: [],
  /*actionHandler: function(state: Game, action: GameAction): GameOutcome {
    switch (action) {
      case DodgeTrap:
        // TODO: Move to seperate function(s) to reuse
        final screen: Screen = state.getScreen();
        final roomScreenState: Null<GameRoomState> = state.tryGetScreenState();
        if (roomScreenState == null) {
          return Invalid;
        }

        final roomState: TrapRoom = roomScreenState.getRoomState(state);
        if (!roomState.activatedTrap && Math.random() >= 0.50) {
          state.player.ModifyHealth(-10);
        }

        roomState.activatedTrap = true;
        return GetNextOutput;
      default:
        return Invalid;
    }
  }*/
};
