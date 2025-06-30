package extensions.trap;

import backend.Extension;
// import backend.Game;
// import backend.GameInfo;
// import backend.Screen;
// import extensions.rooms.Screens;
import extensions.trap.Actions;
import extensions.trap.Rooms;

@:nullSafety(Strict)
final TrapExt: Extension = {
  module: 'extensions.trap',
  actions: [
    DodgeTrap,
  ],
  outcomes: [],
  rooms: [
    TrapRoom,
  ],
  screens: [],
};
