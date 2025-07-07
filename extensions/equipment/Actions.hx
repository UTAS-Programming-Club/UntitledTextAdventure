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
    final roomState: ChestRoomState = state.getRoomState();
    if (!roomState.opened) {
      // TODO: Track and unlock item
    }

    roomState.opened = true;
    return GetNextOutput;
  }
}
