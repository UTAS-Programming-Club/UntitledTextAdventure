package backend;

import backend.Entity;
import backend.Screen;

class GlobalData {
  // public static final rooms: Array<Array<Room>> = [
  //   [
  //   ]
  // ];

  // TODO: Remove, visit count is only used for testing
  private static var mainMenuVisitCount: Int = 0;
  public static final mainMenuScreen = new ActionScreen(
    function (state: GameState) {
      var body: UnicodeString =
        "Untitled text adventure game\n" +
        "----------------------------\n" +
        "By the UTAS Programming Club\n\n" +
        "Currently unimplemented :(";

      if (mainMenuVisitCount > 0) {
        body += "\n\nReload count: " + mainMenuVisitCount;
      }
      mainMenuVisitCount++;

      return body;
    }
  );

  static final gameScreen = new ActionScreen(
    "Game rooms are not currently supported",
    [
      new ScreenAction(
        "Quit",
        ScreenActionType.GotoScreen(mainMenuScreen)
      )
    ]
  );

  static final loadScreen = new ActionScreen(
    "Game loading is not currently supported",
    [
      new ScreenAction(
        "Quit",
        ScreenActionType.GotoScreen(mainMenuScreen)
      )
    ]
  );

  public static final enemyStats: Map<UnicodeString, EntityStats> = [
    "Demon" => new EntityStats(100, 100, 10, 10, 10, 10, 10)
  ];

  public static function Init(): Void {
    mainMenuScreen.Init([
      new ScreenAction(
        "Start Game",
        ScreenActionType.GotoScreen(gameScreen)
      ),
      new ScreenAction(
        "Load Game",
        ScreenActionType.GotoScreen(loadScreen)
      ),
      new ScreenAction(
        "Quit Game",
        ScreenActionType.QuitGame
      )
    ]);
  }
}
