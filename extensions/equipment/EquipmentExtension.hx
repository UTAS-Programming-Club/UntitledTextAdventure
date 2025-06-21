package extensions.equipment;

import backend.Extension;
import backend.GameEnums;
import extensions.equipment.EquipmentInfo;

@:nullSafety(Strict)
final EquipmentExt: Extension = {
  actions: [],
  equipmentObjs: [EquipmentEquipment],
  roomObjs: [],
  screenObjs: []
  // TODO: Make optional?
  /*actionHandler: function(state: Game, action: GameAction): GameOutcome {
    // This extension defines no actions currently
    return Invalid;
  }*/
};
