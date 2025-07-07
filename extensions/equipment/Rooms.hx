package extensions.equipment;

import backend.Action;
import backend.Game;
import backend.GameInfo;
import backend.Room;
import backend.Screen;
import extensions.equipment.Actions;

class ChestRoom extends StatefulRoom<ChestRoomState> {
  final item: GameEquipment;

  public function new(item: GameEquipment) {
    super();
    this.item = item;
  }

  override function getStatefulRoomBody(Game, state: ChestRoomState): UnicodeString {
    var body: UnicodeString = '\n\nThere is a chest which has';

    if (!state.opened) {
      body += ' not';
    }

    body += ' been opened.';

    if (state.opened) {
      body += '\nIt contained a ${item.name}.';
    }

    return body;
  }

  override function getRoomActions(): Array<Action> return [
    new OpenChest('Open Chest'),
  ];
}

class ChestRoomState extends ScreenState {
  public var opened: Bool = false;
}
