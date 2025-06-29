package extensions.rooms;

import backend.Extension;
import extensions.rooms.Actions;
import extensions.rooms.Rooms;
import extensions.rooms.Screens;

// TODO: Merge into CoreGame?
@:nullSafety(Strict)
final RoomsExt: Extension = {
  module: 'extensions.rooms',
  actions: [
    GoNorth,
    GoEast,
    GoSouth,
    GoWest,
  ],
  outcomes: [],
  rooms: [
    UnusedRoom,
    EmptyRoom,
  ],
  screens: [
    GameRoomsScreen,
  ],
};
