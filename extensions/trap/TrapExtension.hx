package extensions.trap;

import backend.Extension;
import extensions.trap.Actions;
import extensions.trap.Rooms;

@:nullSafety(Strict)
final TrapExt: Extension = {
  module: 'extensions.trap',
  actions: [
    DodgeTrap,
  ],
  equipment: [],
  outcomes: [],
  rooms: [
    TrapRoom,
  ],
  screens: [],
};
