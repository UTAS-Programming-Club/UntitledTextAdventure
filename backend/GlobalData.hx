package backend;

import backend.Entity;
import backend.Screen;

class GlobalData {
  // public static final rooms: Array<Array<Room>> = [
  //   [
  //   ]
  // ];

  static final gameScreen = new ActionScreen(
    "Game rooms are not currently supported", []
  );

  static final loadScreen = new ActionScreen(
    "Game loading is not currently supported", []
  );

  public static final mainMenuScreen = new ActionScreen(
    "Untitled text adventure game\n" +
    "----------------------------\n" +
    "By the UTAS Programming Club\n\n" +
    "Currently unimplemented :(",
    [
      new ScreenAction(
        "Start Game",
        ScreenActionType.GotoScreen(gameScreen),
        ScreenAction.AlwaysVisible
      ),
      new ScreenAction(
        "Load Game",
        ScreenActionType.GotoScreen(loadScreen),
        ScreenAction.AlwaysVisible
      ),
      new ScreenAction(
        "Quit Game",
        ScreenActionType.QuitGame,
        ScreenAction.AlwaysVisible
      )
    ]
  );

  public static final enemyStats: Map<UnicodeString, EntityStats> = [
    "Demon" => new EntityStats(100, 100, 10, 10, 10, 10, 10)
  ];
}
