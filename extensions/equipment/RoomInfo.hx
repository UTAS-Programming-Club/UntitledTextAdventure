package extensions.equipment;

import backend.GameInfo;
import extensions.equipment.Rooms;

@:nullSafety(Strict)
final EquipmentRooms: Map<GameRoom, Void -> backend.Room> = [
  Chest => ChestRoom.new
];
