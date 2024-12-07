package backend;

import backend.Entity;

enum abstract RoomType(Int) from Int {
  final Empty;
  final Invalid;
  final HealthChange;
  final CustomChest;
  final Combat;
}

class Room {
  public final type: RoomType;

  // Only set if type != InvalidRoomType
  public final x: Int;
  public final y: Int;

  // Only set if type == HealthChangeRoomType
  // TODO: Check string format, CRoomInfo uses utf-8 but I think this is utf-16
  // public final eventDescription: String;
  // public final eventPercentageChance: Int;

  // Only set if type == CombatRoomType
  // public final enemyCount: Int;
  // TODO: Add actual enemy info with defence and other fields
  // enemies: Array<Enemy>;
  
  public function new(type: RoomType, x: Int, y: Int) {
    this.type = type;
    this.x = x;
    this.y = y;
  }
}
