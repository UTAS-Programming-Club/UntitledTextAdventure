package extensions.rooms;

import backend.Game;
import backend.GameInfo;
import backend.Screen;
import extensions.rooms.Screens;
// TODO: Fix rooms extension depending on other extensions
import extensions.equipment.Rooms.ChestRoom;
import extensions.trap.Rooms.TrapRoom;

/*@:nullSafety(Strict)
function RoomTest(state: Game, screen: Screen): UnicodeString {
  final roomScreenState: GameRoomState = state.getScreenState();
  final x: Int = roomScreenState.x;
  final y: Int = roomScreenState.y;
  final room: GameRoom = state.campaign.rooms[x][y];

  var body: UnicodeString = 'This is the game, you are in Room [${x + 1}, ${y + 1}].';

  // TODO: Fix rooms extension depending on other extensions
  switch (room) {
    case Chest:
      final roomState: ChestRoom = roomScreenState.getRoomState(state);
      body += '\n\nThis is a chest room which has';

      if (!roomState.openedChest) {
        body += ' not';
      }

      body += ' been opened.';
    default:
  }

  return body;
}

@:nullSafety(Strict)
final RoomScreens: Map<GameScreen, Screen> = [
  GameRooms => new StatefulActionScreen(GameRoomState.new, RoomTest, [
    new ScreenAction(OpenChest, "Open Chest", function (state: Game, screen: ActionScreen): Bool {
      final roomScreenState: GameRoomState = state.getScreenState();
      final room: GameRoom = state.campaign.rooms[roomScreenState.x][roomScreenState.y];

      if (room != Chest) {
        return false;
      }

      final roomState: ChestRoom = roomScreenState.getRoomState(state);

      return !roomState.openedChest;
    }),
    new ScreenAction(GotoScreen(PlayerEquipment), "Check Inventory"),
#if testrooms
    new ScreenAction(QuitGame, "Quit Game")
#else
    new ScreenAction(GotoScreen(MainMenu), "Return to main menu")
#end
  ])
];*/
