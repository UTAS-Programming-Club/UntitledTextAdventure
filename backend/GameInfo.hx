package backend;

import backend.Equipment;
import backend.GameEnums;
import backend.Room;
import backend.Screen;

import backend.Extension;

// @:build(backend.macros.TypeGeneration.buildGameMap("EquipmentInfo.hx", "Equipment"))
// @:build(backend.macros.TypeGeneration.buildGameMap("RoomInfo.hx", "Rooms"))
// @:build(backend.macros.TypeGeneration.buildGameMap("ScreenInfo.hx", "Screens"))
@:build(backend.macros.TypeGeneration.buildGameMaps())
class GameInfo {
  public static final Equipment: Map<GameEquipment, Equipment> = [];
  public static final Rooms: Map<GameRoom, Void -> Room> = [];
  public static final Screens: Map<GameScreen, Screen> = [];
}
