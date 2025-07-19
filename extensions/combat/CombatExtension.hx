package extensions.combat;

import backend.Extension;
import extensions.combat.Rooms;
import extensions.combat.Screens;

final CombatExt: Extension = {
  module: 'extensions.combat',
  actions: [],
  equipment: [],
  outcomes: [],
  rooms: [
    CombatRoom,
  ],
  screens: [
    CombatScreen,
  ],
};
