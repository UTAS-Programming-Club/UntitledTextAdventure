package backend;

import backend.Screen;

@:build(backend.macros.TypeGeneration.buildGameEnum("Actions.hx"))
enum GameAction {
}

@:build(backend.macros.TypeGeneration.buildGameEnum("Rooms.hx"))
enum GameRoom {
}

@:build(backend.macros.TypeGeneration.buildGameEnum("Screens.hx"))
enum GameScreen {
}

@:build(backend.macros.TypeGeneration.buildGameMap("ScreenInfo.hx", "Screens"))
class GameInfo {
}

@:build(backend.macros.TypeGeneration.buildGameEnum("Outcomes.hx"))
enum GameOutcome {
}
