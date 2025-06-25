package backend;

import backend.Action;
// import backend.Equipment;
import backend.Outcome;
// import backend.Room;
import backend.Screen;

// Types able to hold any item without a constructor
// For runtime usage, the actual types are typically required
typedef GameAction = Class<Action>;
typedef GameOutcome = Class<Outcome>;
typedef GameScreen = EnumValue;

// TODO: Either namespace game data per campaign or remove support for multiple campaigns in a single build

// @:build(backend.macros.TypeGeneration.buildGameEnum("Actions.hx"))
/*enum GameAction {
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Equipments.hx"))
enum GameEquipment {
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Rooms.hx"))
enum GameRoom {
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Outcomes.hx"))
enum GameOutcome {
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Screens.hx"))
enum GameScreen {
}

// @:build(backend.macros.TypeGeneration.buildGameMap("RoomInfo.hx", "Rooms"))
// @:build(backend.macros.TypeGeneration.buildGameMap("ScreenInfo.hx", "Screens"))
// @:build(backend.macros.TypeGeneration.buildGameMap("EquipmentInfo.hx", "Equipment"))
class GameInfo {
  public static final Rooms: Map<GameRoom, Void -> Room> = [];
  public static final Screens: Map<GameScreen, Screen> = [];
  public static final Equipment: Map<GameEquipment, Equipment> = [];
}*/
