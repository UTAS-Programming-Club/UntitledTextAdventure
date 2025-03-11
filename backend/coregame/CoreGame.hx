package backend.coregame;

import backend.Game;
import backend.Screen;

enum CoreRooms {
}

enum CoreScreens {
  MainMenu;
}

final CoreScreenInfo: Map<CoreScreens, Screen> = [
  MainMenu => new ActionScreen(
      "Untitled text adventure game\n"
    + "----------------------------\n"
    + "By the UTAS Programming Club\n\n"
    + "Currently unimplemented :(",
    [
      new ScreenAction(
        "Quit Game",
        Actions.QuitGame,
      )
    ]
  )
];

private final CoreGame: Extension = {
  actions: CoreActions,
  equipment: [],
  rooms: CoreRooms,
  screens: CoreScreenInfo
};
