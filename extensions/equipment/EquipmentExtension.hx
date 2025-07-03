package extensions.equipment;

import backend.Extension;
import extensions.equipment.Equipment;
import extensions.equipment.Rooms;

@:nullSafety(Strict)
final EquipmentExt: Extension = {
  module: 'extensions.equipment',
  actions: [],
  equipment: [
    HeadNone,
    UpperBodyRags,
    HandsNone,
    LowerBodyRags,
    FeetNone,
    PrimaryWeaponFist,
    SecondaryWeaponNone,
  ],
  outcomes: [],
  screens: [
    ChestRoom,
  ],
  /*actionHandler: function(state: Game, action: GameAction): GameOutcome {
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
  }*/
};
