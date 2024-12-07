package backend;

import backend.Entity;
import backend.Screen;

class GlobalData {
  // public static final rooms: Array<Array<Room>> = [
  //   [
  //   ]
  // ];

  static final test = new ActionScreen(
    "This is a test", []
  );

  public static final mainMenu = new ActionScreen(
    "Untitled text adventure game\n" +
    "----------------------------\n" +
    "By the UTAS Programming Club\n\n" +
    "Currently unimplemented :(",
    [
      new ScreenAction(
        "Start Game",
        ScreenActionType.GotoScreen(test),
        function(state) return true
      )
    ]
  );

  public static final enemyStats: Map<UnicodeString, EntityStats> = [
    "Demon" => new EntityStats(100, 100, 10, 10, 10, 10, 10)
  ];
}
