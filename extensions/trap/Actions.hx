package extensions.trap;

import backend.Action;
import backend.coregame.Outcomes;
import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.rooms.Screens;
import extensions.trap.Rooms;

class DodgeTrap extends Action {
  override function isVisible(state: Game, ActionScreen): Bool {
    final roomScreenState: GameRoomState = state.getScreenState();
    final room: GameRoom = state.campaign.rooms[roomScreenState.x][roomScreenState.y];

    if (room != TrapRoom) {
      return false;
    }

    final roomState: TrapRoomState = roomScreenState.getRoomState(state);

    return !roomState.activatedTrap;
  }

  function trigger(state: Game): GameOutcome {
    // TODO: Make this easier for rooms/stateful rooms?
    final screen: Screen = state.getScreen();
    final roomScreenState: Null<GameRoomState> = state.tryGetScreenState();
    if (roomScreenState == null) {
      return Invalid;
    }

    final roomState: TrapRoomState = roomScreenState.getRoomState(state);
    if (!roomState.activatedTrap && Math.random() >= 0.50) {
      state.player.ModifyHealth(-10);
    }

    roomState.activatedTrap = true;
    return GetNextOutput;
  }
}
