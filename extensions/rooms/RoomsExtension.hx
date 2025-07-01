package extensions.rooms;

import backend.Extension;
import extensions.rooms.Actions;
import extensions.rooms.Rooms;

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
  screens: [
    UnusedRoom,
    EmptyRoom,
  ],
};
