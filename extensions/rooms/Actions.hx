package extensions.rooms;

import backend.Action;
import backend.coregame.Outcomes;
import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.rooms.Rooms;
import extensions.rooms.Screens;

class GoNorth extends Action {
  override function isVisible(state: Game, ActionScreen): Bool {
    final roomScreenState: GameRoomState = state.getScreenState();
    return roomScreenState.y < state.campaign.rooms.length - 1 &&
           state.campaign.rooms[roomScreenState.x][roomScreenState.y + 1] != UnusedRoom;
  };

  // TODO: Can getScreenState be used directly?
  function trigger(state: Game): GameOutcome {
    final roomScreenState: Null<GameRoomState> = state.tryGetScreenState();
    if (roomScreenState == null) {
      return Invalid;
    }

    roomScreenState.changeRoom(state, roomScreenState.x, roomScreenState.y + 1);
    return GetNextOutput;
  }
}

class GoEast extends Action {
  override function isVisible(state: Game, ActionScreen): Bool {
    final roomScreenState: GameRoomState = state.getScreenState();
    return roomScreenState.x > 0 &&
           state.campaign.rooms[roomScreenState.x - 1][roomScreenState.y] != UnusedRoom;
  }

  // TODO: Can getScreenState be used directly?
  function trigger(state: Game): GameOutcome {
    final roomScreenState: Null<GameRoomState> = state.tryGetScreenState();
    if (roomScreenState == null) {
      return Invalid;
    }

    roomScreenState.changeRoom(state, roomScreenState.x - 1, roomScreenState.y);
    return GetNextOutput;
  }
}

class GoSouth extends Action {
  override function isVisible(state: Game, ActionScreen): Bool {
    final roomScreenState: GameRoomState = state.getScreenState();
    return roomScreenState.y > 0 &&
            state.campaign.rooms[roomScreenState.x][roomScreenState.y - 1] != UnusedRoom;
  }

  // TODO: Can getScreenState be used directly?
  function trigger(state: Game): GameOutcome {
    final roomScreenState: Null<GameRoomState> = state.tryGetScreenState();
    if (roomScreenState == null) {
      return Invalid;
    }

    roomScreenState.changeRoom(state, roomScreenState.x, roomScreenState.y - 1);
    return GetNextOutput;
  }
}

class GoWest extends Action {
  override function isVisible(state: Game, ActionScreen): Bool {
    final roomScreenState: GameRoomState = state.getScreenState();
    return roomScreenState.x < state.campaign.rooms.length - 1 &&
            state.campaign.rooms[roomScreenState.x + 1][roomScreenState.y] != UnusedRoom;
  }

  // TODO: Can getScreenState be used directly?
  function trigger(state: Game): GameOutcome {
    final roomScreenState: Null<GameRoomState> = state.tryGetScreenState();
    if (roomScreenState == null) {
      return Invalid;
    }

    roomScreenState.changeRoom(state, roomScreenState.x + 1, roomScreenState.y);
    return GetNextOutput;
  }
}
