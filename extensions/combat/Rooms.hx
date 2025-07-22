package extensions.combat;

import backend.Equipment;
import backend.Game;
import backend.Room;

import extensions.equipment.Equipment;
import extensions.combat.Combat;
import extensions.combat.Screens;

class CombatRoom extends StatefulRoom<CombatRoomState> {
  override function onStatefulEntry(state: Game, roomState: CombatRoomState): Void {
    if (!roomState.won) {
      roomState.phase = WaitingForInput;
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

  public var phase: CombatPhase = WaitingForInput;
  public var currentWeapon: Equipment = WeaponNone;
  public var currentEnemyNumber: Int = -1;

  function isCompleted(): Bool return won;
  function requireCompleted(): Bool return true;
}
