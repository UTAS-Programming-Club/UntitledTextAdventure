package extensions.rooms;

import backend.Action;
import backend.coregame.Outcomes;
import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.rooms.Rooms;

class GoNorth extends Action {
  override function isVisible(state: Game, ActionScreen): Bool {
    return state.player.y < state.campaign.rooms.length - 1 &&
           state.campaign.rooms[state.player.x][state.player.y + 1] != UnusedRoom;
  };

  function trigger(state: Game): GameOutcome {
    state.gotoRoom(state.player.x, state.player.y + 1);
    return GetNextOutput;
  }
}

class GoEast extends Action {
  override function isVisible(state: Game, ActionScreen): Bool {
    return state.player.x > 0 &&
           state.campaign.rooms[state.player.x - 1][state.player.y] != UnusedRoom;
  }

  function trigger(state: Game): GameOutcome {
    state.gotoRoom(state.player.x - 1, state.player.y);
    return GetNextOutput;
  }
}

class GoSouth extends Action {
  override function isVisible(state: Game, ActionScreen): Bool {
    return state.player.y > 0 &&
            state.campaign.rooms[state.player.x][state.player.y - 1] != UnusedRoom;
  }

  function trigger(state: Game): GameOutcome {
    state.gotoRoom(state.player.x, state.player.y - 1);
    return GetNextOutput;
  }
}

class GoWest extends Action {
  override function isVisible(state: Game, ActionScreen): Bool {
    return state.player.x < state.campaign.rooms.length - 1 &&
            state.campaign.rooms[state.player.x + 1][state.player.y] != UnusedRoom;
  }

  function trigger(state: Game): GameOutcome {
    state.gotoRoom(state.player.x + 1, state.player.y);
    return GetNextOutput;
  }
}
