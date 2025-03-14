package extensions.untitledtextadventure;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

final UTAScreenInfo: Map<GameScreen, Screen> = [
  GameRooms => new ActionScreen("This is the game", [
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu", function (state: Game) {
      state.currentScreen = MainMenu;
      return GetNextOutput;
    })
  ]),
];
