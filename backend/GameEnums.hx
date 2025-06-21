package backend;

// @:build(backend.macros.TypeGeneration.buildGameEnum("Actions.hx"))
enum GameAction {
  StartGame;
  GotoScreen(screen: GameScreen);
  GotoPreviousScreen;
  QuitGame;
  GoNorth;
  GoEast;
  GoSouth;
  GoWest;
  DodgeTrap;
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Equipments.hx"))
enum GameEquipment {
  HeadNone;
  UpperBodyRags;
  HandsNone;
  LowerBodyRags;
  FeetNone;
  PrimaryWeaponFist;
  SecondaryWeaponNone;
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Outcomes.hx"))
enum GameOutcome {
  Invalid;
  GetNextOutput;
  QuitGame;
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Rooms.hx"))
enum GameRoom {
  Unused;
  Empty;
  Trap;
}

// @:build(backend.macros.TypeGeneration.buildGameEnum("Screens.hx"))
enum GameScreen {
  MainMenu;
  GameRooms;
  Load;
  PlayerEquipment;
}
