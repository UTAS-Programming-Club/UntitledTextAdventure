package backend;

import backend.Equipment;
import backend.Room;
import backend.Screen;

// TODO: Either namespace game data per campaign or remove support for multiple campaigns in a single build

@:build(backend.macros.TypeGeneration.buildGameEnum('Actions.hx', 'Action'))
enum GameAction {
}

// TODO: Split by equipment type
@:build(backend.macros.TypeGeneration.buildGameEnum('EquipmentTypes.hx', 'EquipmentHead'))
enum GameEquipmentHead {
}

@:build(backend.macros.TypeGeneration.buildGameEnum('EquipmentTypes.hx', 'EquipmentUpperBody'))
enum GameEquipmentUpperBody {
}

@:build(backend.macros.TypeGeneration.buildGameEnum('EquipmentTypes.hx', 'EquipmentHand'))
enum GameEquipmentHands {
}

@:build(backend.macros.TypeGeneration.buildGameEnum('EquipmentTypes.hx', 'EquipmentLowerBody'))
enum GameEquipmentLowerBody {
}

@:build(backend.macros.TypeGeneration.buildGameEnum('EquipmentTypes.hx', 'EquipmentFeet'))
enum GameEquipmentFeet {
}

@:build(backend.macros.TypeGeneration.buildGameEnum('EquipmentTypes.hx', 'EquipmentPrimaryWeapon'))
enum GameEquipmentPrimaryWeapon {
}

@:build(backend.macros.TypeGeneration.buildGameEnum('EquipmentTypes.hx', 'EquipmentSecondaryWeapon'))
enum GameEquipmentSecondaryWeapon {
}


@:build(backend.macros.TypeGeneration.buildGameEnum('Rooms.hx', 'Room'))
enum GameRoom {
}

@:build(backend.macros.TypeGeneration.buildGameEnum('Outcomes.hx', 'Outcome'))
enum GameOutcome {
}

@:build(backend.macros.TypeGeneration.buildGameEnum('Screens.hx', 'Screen'))
enum GameScreen {
}

@:build(backend.macros.TypeGeneration.buildGameMap('RoomInfo.hx', 'Rooms'))
@:build(backend.macros.TypeGeneration.buildGameMap('ScreenInfo.hx', 'Screens'))
@:build(backend.macros.TypeGeneration.buildGameMap('EquipmentInfo.hx', 'EquipmentHead'))
@:build(backend.macros.TypeGeneration.buildGameMap('EquipmentInfo.hx', 'EquipmentUpperBody'))
@:build(backend.macros.TypeGeneration.buildGameMap('EquipmentInfo.hx', 'EquipmentHands'))
@:build(backend.macros.TypeGeneration.buildGameMap('EquipmentInfo.hx', 'EquipmentLowerBody'))
@:build(backend.macros.TypeGeneration.buildGameMap('EquipmentInfo.hx', 'EquipmentFeet'))
@:build(backend.macros.TypeGeneration.buildGameMap('EquipmentInfo.hx', 'EquipmentPrimaryWeapon'))
@:build(backend.macros.TypeGeneration.buildGameMap('EquipmentInfo.hx', 'EquipmentSecondaryWeapon'))
class GameInfo {
  public static final Rooms: Map<GameRoom, Void -> Room> = [];
  public static final Screens: Map<GameScreen, Screen> = [];
  public static final EquipmentHead: Map<GameEquipmentHead, Equipment> = [];
  public static final EquipmentUpperBody: Map<GameEquipmentUpperBody, Equipment> = [];
  public static final EquipmentHands: Map<GameEquipmentHands, Equipment> = [];
  public static final EquipmentLowerBody: Map<GameEquipmentLowerBody, Equipment> = [];
  public static final EquipmentFeet: Map<GameEquipmentFeet, Equipment> = [];
  public static final EquipmentPrimaryWeapon: Map<GameEquipmentPrimaryWeapon, Equipment> = [];
  public static final EquipmentSecondaryWeapon: Map<GameEquipmentSecondaryWeapon, Equipment> = [];
}
