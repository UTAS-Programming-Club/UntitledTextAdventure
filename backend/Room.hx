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

abstract class Room extends ActionScreen {
  function getRoomBody(state: Game): UnicodeString return "";

  // TODO: Move all the strings to extensions/campaigns
  function getBody(state: Game): UnicodeString {
    final x: Int = state.player.x;
    final y: Int = state.player.y;
    final room: GameRoom = state.campaign.rooms[x][y];

    var body: UnicodeString = 'This is the game, you are in Room [${x + 1}, ${y + 1}].';
    body += getRoomBody(state);

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

  function getStatefulRoomBody(state: Game, screenState: T): UnicodeString return "";

  override function getRoomBody(state: Game): UnicodeString return getStatefulRoomBody(state, state.getRoomState());
}
