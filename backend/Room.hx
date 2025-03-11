package backend;

import backend.Helpers;
import haxe.Constraints;

enum RoomDirection {
  North;
  East;
  South;
  West;
}

@:nullSafety(Strict)
abstract class Room {
  public function new(): Void {
  }

  abstract public function GetBody(state: GameState): String;

  public function GetAllowedDirections(state: GameState): Array<RoomDirection> {
    return Type.allEnums(RoomDirection);
  }
}

@:generic
@:nullSafety(Strict)
abstract class StatefulRoom<T : Constructible<Void->Void> & BasicRoomState> extends Room {
  public function CreateState(): T {
    return new T();
  }

  // TODO: Support restoring state on load
}

@:nullSafety(Strict)
class BasicRoomState {
  // Stateful rooms can store additional state at runtime, but,
  // only completed is kept in passwords to keep them short
  public var completed: Bool = false;

  public function new(): Void {
  }
}


@:nullSafety(Strict)
class EmptyRoom extends Room {
  public function GetBody(state: GameState) {
    return "";
  }
}

/*@:nullSafety(Strict)
class TrapRoom extends StatefulRoom<BasicRoomState> {
  public final lowerAgility: UInt;
  public final upperAgility: UInt;
  public final damage: UInt;
  public final exitDirection: RoomDirection;

  public function new(lowerAgility: UInt, upperAgility: UInt, damage: UInt, exitDirection: RoomDirection) {
    super();

    this.lowerAgility = lowerAgility;
    this.upperAgility = upperAgility;
    this.damage = damage;
    this.exitDirection = exitDirection;
  }

  public function GetBody(state: GameState) {
    final roomState: BasicRoomState = state.GetCurrentRoomState();
    if (!roomState.completed) {
      return "You have come across a trap.";
    }

    return "";
  }

  public override function GetAllowedDirections(state: GameState): Array<RoomDirection> {
    final roomState: BasicRoomState = state.GetCurrentRoomState();

    final possibleDirections: Array<RoomDirection> = Type.allEnums(RoomDirection);
    if (!roomState.completed) {
      possibleDirections.remove(exitDirection);
    }

    return possibleDirections;
  }
}*/

// CustomChest(...);
// Combat(enemies: Array<Enemy>);
