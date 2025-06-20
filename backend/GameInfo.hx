package backend;

import backend.Equipment;
import backend.GameEnums;
import backend.Room;
import backend.Screen;


// @:build(backend.macros.TypeGeneration.buildGameMap("EquipmentInfo.hx", "Equipment"))
// @:build(backend.macros.TypeGeneration.buildGameMap("RoomInfo.hx", "Rooms"))
// @:build(backend.macros.TypeGeneration.buildGameMap("ScreenInfo.hx", "Screens"))
class GameInfo {
  public static final Equipment: Map<GameEquipment, Equipment> = [];
  public static final Rooms: Map<GameRoom, Void -> Room> = [];
  public static final Screens: Map<GameScreen, Screen> = [
    MainMenu => new ActionScreen(
    function(state: Game, Screen): UnicodeString {
      return state.campaign.mainMenu;
    }, [
      new ScreenAction(QuitGame, "Quit Game")
    ])
  ];
}


@:build(backend.macros.TypeGeneration.macro3())
enum Test3 {
}
