package extensions.equipment;

import backend.Extension;
import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.equipment.Rooms;
import extensions.rooms.Screens;

@:nullSafety(Strict)
final EquipmentExt: Extension = {
  actionHandler: function(state: Game, action: GameAction): GameOutcome {
    switch (action) {
      case OpenChest:
        // TODO: Move to seperate function(s) to reuse
        final screen: Screen = state.getScreen();
        final roomScreenState: Null<GameRoomState> = state.tryGetScreenState();
        if (roomScreenState == null) {
          return Invalid;
        }

        final room: GameRoom = state.campaign.rooms[roomScreenState.x][roomScreenState.y];
        final roomState: ChestRoom = roomScreenState.getRoomState(state);
        switch (room) {
          case Chest if (!roomState.openedChest):
            // TODO: Track and unlock item
          default:
        }

        roomState.openedChest = true;
        return GetNextOutput;
      default:
        return Invalid;
    }
  }
};
