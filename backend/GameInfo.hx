package backend;

import backend.Screen;

@:build(backend.macros.TypeGeneration.buildGameEnum("Action"))
enum GameAction {
}

@:build(backend.macros.TypeGeneration.buildGameEnum("Screen"))
enum GameScreen {
}

@:build(backend.macros.TypeGeneration.buildGameMap("Screens"))
class GameInfo {
}
