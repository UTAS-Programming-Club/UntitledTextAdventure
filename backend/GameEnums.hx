package backend;

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

// @:build(backend.macros.TypeGeneration.buildGameEnum("Outcomes.hx"))
enum GameOutcome {
  Invalid;
  GetNextOutput;
  QuitGame;
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Rooms.hx"))
enum GameRoom {
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Screens.hx"))
enum GameScreen {
  MainMenu;
  Test;
  Test2;
}
