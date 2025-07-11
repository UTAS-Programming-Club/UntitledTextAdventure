package extensions.equipment;

import backend.Action;
import backend.coregame.Outcomes;
import backend.Game;
import backend.GameInfo;
import extensions.equipment.Rooms;

class OpenChest extends Action {
  override function isVisible(state: Game): Bool {
    final room: GameRoom = state.campaign.rooms[state.player.x][state.player.y];
    if (!(room is ChestRoom)) {
      return false;
    }

    final roomState: ChestRoomState = state.getRoomState();
    return !roomState.opened;
  }

  function onTrigger(state: Game): GameOutcome {
    // onTrigger should only run after isVisible which does this check properly
    final room: ChestRoom = cast(state.campaign.rooms[state.player.x][state.player.y], ChestRoom);
    final roomState: ChestRoomState = state.getRoomState();
    if (roomState.opened) {
      return Invalid;
    }

    roomState.opened = true;
    state.player.unlockItem(state.campaign, room.item);

    return GetNextOutput;
  }
}
