package backend;

import haxe.Constraints;

import backend.Action;
// TODO: Fix backend depending on coregame extension?
import backend.coregame.Actions;
import backend.coregame.Screens;
import backend.Game;
import backend.GameInfo;
import backend.Screen;
// TODO: Fix backend depending on rooms extension
import extensions.rooms.Actions;
// TODO: Fix backend depending on traps extension
import extensions.trap.Actions;
import extensions.trap.Rooms;

abstract class Room extends ActionScreen {
  // TODO: Move all the strings to extensions/campaigns
  function getBody(state: Game): UnicodeString {
    final x: Int = state.player.x;
    final y: Int = state.player.y;
    final room: GameRoom = state.campaign.rooms[x][y];

    var body: UnicodeString = 'This is the game, you are in Room [${x + 1}, ${y + 1}].';

    // TODO: Fix backend depending on traps extension
    if (room == TrapRoom) {
      final trapRoomState: TrapRoomState = state.getRoomState();
      body += '\n\nThis is a trap room which has';

      if (!trapRoomState.activatedTrap) {
        body += ' not';
      }

      body += ' been triggered.';
    }

    return body;
  }

  // TODO: Move all the actions to extensions/campaigns
  function getAllActions(): Array<Action> return [
    new GoNorth("Go North"),
    new GoEast("Go East"),
    new GoSouth("Go South"),
    new GoWest("Go West"),
    // TODO: Fix backend depending on traps extension
    new DodgeTrap("Dodge Trap"),
    new GotoScreen(PlayerEquipmentScreen, "Check Inventory"),
#if testrooms
    new Quit("Quit Game")
#else
    new GotoScreen(MainMenuScreen, "Return to main menu")
#end
  ];
}

// Keep in sync with StatefulActionScreen in Screen.hx
@:generic
abstract class StatefulRoom<T : ScreenState & Constructible<Void -> Void>> extends Room {
  override function hasState(): Bool return true;
  override function createState(): T return new T();

  // TODO: Fix
  // abstract function getStatefulBody(state: Game, screenState: T): UnicodeString;

  // function getBody(state: Game): UnicodeString return getStatefulBody(state, state.getScreenState());
}
