package extensions.combat;

import backend.Extension;
import extensions.combat.Actions;
import extensions.combat.Rooms;
import extensions.combat.Screens;

final CombatExt: Extension = {
  module: 'extensions.combat',
  actions: [
    AttackEnemy,
  ],
  equipment: [],
  outcomes: [],
  rooms: [
    CombatRoom,
  ],
  screens: [
    CombatScreen,
  ],
};
