package backend.coregame;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

final CoreScreenInfo: Map<GameScreen, Screen> = [
  MainMenu => new ActionScreen(function(state: Game) {
      return state.campaign.name + "\n"
           + "----------------------------\n"
           + "By " + state.campaign.author + "\n"
           + "\n"
           + "Currently unimplemented :(";
    }, [
    new ScreenAction(StartGame, "Start Game"),
    new ScreenAction(GotoScreen(Load), "Load Game"),
    new ScreenAction(QuitGame, "Quit Game")
  ]),
  GameRooms => new ActionScreen("This is the game", [
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
  ]),
  Load => new ActionScreen("Loading is not currently supported", [
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
  ])
];
