package backend;

import backend.Equipment;
import backend.Room;
import backend.Screen;

// TODO: Either namespace game data per campaign or remove support for multiple campaigns in a single build

// @:build(backend.macros.TypeGeneration.buildGameEnum("Actions.hx"))
enum GameAction {
  StartGame;
  GotoScreen(screen: GameScreen);
  GotoPreviousScreen;
  QuitGame;
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Equipments.hx"))
enum GameEquipment {
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Rooms.hx"))
enum GameRoom {
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Outcomes.hx"))
enum GameOutcome {
  Invalid;
  GetNextOutput;
  QuitGame;
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Screens.hx"))
enum GameScreen {
  MainMenu;
}

// @:build(backend.macros.TypeGeneration.buildGameMap("RoomInfo.hx", "Rooms"))
// @:build(backend.macros.TypeGeneration.buildGameMap("ScreenInfo.hx", "Screens"))
// @:build(backend.macros.TypeGeneration.buildGameMap("EquipmentInfo.hx", "Equipment"))
class GameInfo {
  public static final Rooms: Map<GameRoom, Void -> Room> = [];
  public static final Screens: Map<GameScreen, Screen> = [
    MainMenu => new ActionScreen(
    function(state: Game, Screen): UnicodeString {
      return state.campaign.mainMenu;
    }, [
      new ScreenAction(QuitGame, "Quit Game")
    ])
  ];
  public static final Equipment: Map<GameEquipment, Equipment> = [];
}
