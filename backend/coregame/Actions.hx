package backend.coregame;

import backend.Action;
import backend.coregame.Outcomes;
import backend.coregame.Rooms;
import backend.Game;
import backend.GameInfo;
import backend.Screen;

class StartGame extends Action {
  function onTrigger(state: Game): GameOutcome {
    state.startGame();
    return GetNextOutput;
  }
}

class GotoScreen extends Action {
  private final screen: GameScreen;

  public function new(screen: GameScreen, title: UnicodeString) {
    super(title);
    this.screen = screen;
  }

  function onTrigger(state: Game): GameOutcome {
    state.gotoScreen(screen);
    return GetNextOutput;
  }
}

class GotoPreviousScreen extends Action {
  function onTrigger(state: Game): GameOutcome {
    state.gotoScreen(state.previousScreen);
    return GetNextOutput;
  }
}

class Quit extends Action {
  function onTrigger(state: Game): GameOutcome {
    return QuitGame;
  }
}


class GoNorth extends Action {
  override function isVisible(state: Game): Bool {
    return state.player.y < state.campaign.rooms.length - 1 &&
           state.campaign.rooms[state.player.x][state.player.y + 1] != UnusedRoom;
  };

  function onTrigger(state: Game): GameOutcome {
    state.gotoRoom(state.player.x, state.player.y + 1);
    return GetNextOutput;
  }
}

class GoEast extends Action {
  override function isVisible(state: Game): Bool {
    return state.player.x > 0 &&
           state.campaign.rooms[state.player.x - 1][state.player.y] != UnusedRoom;
  }

  function onTrigger(state: Game): GameOutcome {
    state.gotoRoom(state.player.x - 1, state.player.y);
    return GetNextOutput;
  }
}

class GoSouth extends Action {
  override function isVisible(state: Game): Bool {
    return state.player.y > 0 &&
            state.campaign.rooms[state.player.x][state.player.y - 1] != UnusedRoom;
  }

  function onTrigger(state: Game): GameOutcome {
    state.gotoRoom(state.player.x, state.player.y - 1);
    return GetNextOutput;
  }
}

class GoWest extends Action {
  override function isVisible(state: Game): Bool {
    return state.player.x < state.campaign.rooms.length - 1 &&
            state.campaign.rooms[state.player.x + 1][state.player.y] != UnusedRoom;
  }

  function onTrigger(state: Game): GameOutcome {
    state.gotoRoom(state.player.x + 1, state.player.y);
    return GetNextOutput;
  }
}
