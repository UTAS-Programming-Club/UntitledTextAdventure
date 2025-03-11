package backend;

import backend.Screen;

@:build(backend.Macros.buildGameEnum("Actions"))
enum GameActions {
}

@:build(backend.Macros.buildGameEnum("Screens"))
enum GameScreens {
}

@:build(backend.Macros.buildGameMap("ScreenInfo"))
class GameInfo {
}

// @:build(backend.Macros.buildGameMap("ScreenInfo"))
// final GameScreenInfo: Map<GameScreens, Screen> = [
//   MainMenu => new Screen("This is a test", [])
// ];
