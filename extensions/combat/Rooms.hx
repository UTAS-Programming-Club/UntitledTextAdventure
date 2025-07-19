package extensions.combat;

import backend.Game;
import backend.Room;
import extensions.combat.Screens;

class CombatRoom extends StatefulRoom<CombatRoomState> {
  override function onStatefulEntry(state: Game, roomState: CombatRoomState): Void {
    if (!roomState.won) {
      final previousX = Std.int(state.previousRoom % state.campaign.rooms.length);
      final previousY = Std.int(state.previousRoom / state.campaign.rooms.length);
      state.gotoRoom(previousX, previousY);
      state.gotoScreen(CombatScreen);
    }
  }

  function getStatefulRoomBody(Game, CombatRoomState): UnicodeString return '';

  function getName(): UnicodeString return 'Combat';
  // TODO: Add symbol
  function getMapSymbol(): UnicodeString return '';
}

class CombatRoomState extends RoomState {
  public var won: Bool = false;

  function isCompleted(): Bool return won;
  function requireCompleted(): Bool return true;
}
