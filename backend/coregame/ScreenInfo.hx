package backend.coregame;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

@:nullSafety(Strict)
final CoreScreens: Map<GameScreen, Screen> = [
  MainMenu => new ActionScreen(function(state: Game, Screen): UnicodeString {
      return state.campaign.mainMenu;
    }, [
    new ScreenAction(StartGame, "Start Game"),
    new ScreenAction(GotoScreen(Load), "Load Game"),
    new ScreenAction(QuitGame, "Quit Game")
  ]),
  Load => new ActionScreen("Loading is not currently supported", [
    new ScreenAction(GotoScreen(MainMenu), "Return to Main Menu")
  ])
];
