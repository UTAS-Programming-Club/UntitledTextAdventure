package backend.coregame;

import backend.Game;
import backend.GameInfo;
import backend.Screen;

// TODO: Avoid duplication of GotoScreen parameter and currentScreen assignment
final CoreScreenInfo: Map<GameScreen, Screen> = [
  MainMenu => new ActionScreen(function(state: Game): UnicodeString {
      return state.campaign.mainMenu;
    }, [
    new ScreenAction(StartGame, "Start Game", function(state: Game): GameOutcome {
      state.currentScreen = state.campaign.gameScreen;
      return GetNextOutput;
    }),
    new ScreenAction(GotoScreen(Load), "Load Game", function(state: Game): GameOutcome {
      state.currentScreen = Load;
      return GetNextOutput;
    }),
    new ScreenAction(QuitGame, "Quit Game", function(state: Game): GameOutcome {
      return QuitGame;
    })
  ]),
  Load => new ActionScreen("Loading is not currently supported", [
    new ScreenAction(GotoScreen(MainMenu), "Return to Main Menu", function(state: Game): GameOutcome {
      state.currentScreen = MainMenu;
      return GetNextOutput;
    })
  ])
];
