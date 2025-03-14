package backend.coregame;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

// TODO: Avoid duplication of GotoScreen parameter and currentScreen assignment
final CoreScreenInfo: Map<GameScreen, Screen> = [
  MainMenu => new ActionScreen(function(state: Game) {
      return state.campaign.name + "\n"
           + "----------------------------\n"
           + "By " + state.campaign.author + "\n"
           + "\n"
           + "Currently unimplemented :(";
    }, [
    new ScreenAction(StartGame, "Start Game", function(state: Game) {
      // TODO: Add way of accessing game without hardcoding here
      // Put in campaign?
      return GetNextOutput;
    }),
    new ScreenAction(GotoScreen(Load), "Load Game", function(state: Game) {
      state.currentScreen = Load;
      return GetNextOutput;
    }),
    new ScreenAction(QuitGame, "Quit Game", function(state: Game) {
      return QuitGame;
    })
  ]),
  Load => new ActionScreen("Loading is not currently supported", [
    new ScreenAction(GotoScreen(MainMenu), "Return to Main Menu", function(state: Game) {
      state.currentScreen = MainMenu;
      return GetNextOutput;
    })
  ])
];
