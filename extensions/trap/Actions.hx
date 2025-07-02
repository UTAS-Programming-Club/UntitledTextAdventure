package extensions.trap;

import backend.Action;
import backend.coregame.Outcomes;
import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.trap.Rooms;

class DodgeTrap extends Action {
  override function isVisible(state: Game): Bool {
    final room: GameRoom = state.campaign.rooms[state.player.x][state.player.y];

    if (room != TrapRoom) {
      return false;
    }

    final roomState: TrapRoomState = state.getRoomState();

    return !roomState.activatedTrap;
  }

  function onTrigger(state: Game): GameOutcome {
    final roomState: TrapRoomState = state.getRoomState();
    if (!roomState.activatedTrap && Math.random() >= 0.50) {
      state.player.ModifyHealth(-10);
    }

    roomState.activatedTrap = true;
    return GetNextOutput;
  }
}
