package extensions.healing;

import backend.Extension;
import extensions.healing.Actions;
import extensions.healing.Rooms;

@:nullSafety(Strict)
final HealingExt: Extension = {
  module: 'extensions.healing',
  actions: [
    HealPlayer,
  ],
  equipment: [],
  outcomes: [],
  rooms: [
    HealingRoom,
  ],
  screens: [],
};
