package backend;

import backend.saving.Helpers;
import backend.Campaign;
import backend.Equipment;
import backend.GameInfo;
import backend.Save;

class Player {
  public var health(default, null): Int = 0;  // Must be in [0, 100]
  public var stamina(default, null): Int = 0; // Must be in [0, 100]

  // TODO: Avoid EnumValue
  private var equippedKeys: Map<EquipmentSlot, EnumValue>;

  public var head(default, null): Equipment;
  public var upperBody(default, null): Equipment;
  public var hands(default, null): Equipment;
  public var lowerBody(default, null): Equipment;
  public var feet(default, null): Equipment;
  public var primaryWeapon(default, null): Equipment;
  public var secondaryWeapon(default, null): Equipment;

  public function new(campaign: Campaign) {
    equippedKeys = [
      Head            => campaign.initialHead,
      UpperBody       => campaign.initialUpperBody,
      Hands           => campaign.initialHands,
      LowerBody       => campaign.initialLowerBody,
      Feet            => campaign.initialFeet,
      PrimaryWeapon   => campaign.initialPrimaryWeapon,
      SecondaryWeapon => campaign.initialSecondaryWeapon
    ];
    head            = Equipment.Get(Head, campaign.initialHead);
    upperBody       = Equipment.Get(UpperBody, campaign.initialUpperBody);
    hands           = Equipment.Get(Hands, campaign.initialHands);
    lowerBody       = Equipment.Get(LowerBody, campaign.initialLowerBody);
    feet            = Equipment.Get(Feet, campaign.initialFeet);
    primaryWeapon   = Equipment.Get(Weapon, campaign.initialPrimaryWeapon);
    secondaryWeapon = Equipment.Get(Weapon, campaign.initialSecondaryWeapon);
  }

  public function Reset(campaign: Campaign): Void {
    health = 100;
    stamina = 100;

    equippedKeys = [
      Head            => campaign.initialHead,
      UpperBody       => campaign.initialUpperBody,
      Hands           => campaign.initialHands,
      LowerBody       => campaign.initialLowerBody,
      Feet            => campaign.initialFeet,
      PrimaryWeapon   => campaign.initialPrimaryWeapon,
      SecondaryWeapon => campaign.initialSecondaryWeapon
    ];
    updateEquipped();
}


  // TODO: End game on 0 health
  // change must be in [0, 100]
  public function ModifyHealth(change: Int): Void {
    var newHealth: Int = health + change;
    if (newHealth < 0) {
      newHealth = 0;
    } else if (newHealth > 100) {
      newHealth = 100;
    }

      health = newHealth;
  }


  private function getEquippedKey(slot: EquipmentSlot): EnumValue {
    final key: Null<EnumValue> = equippedKeys[slot];
      if (key == null) {
        throw 'Invalid equipment slot: $slot.';
      }

    return key;
  }

  private function updateEquipped() {
    head            = Equipment.Get(Head, getEquippedKey(Head));
    upperBody       = Equipment.Get(UpperBody, getEquippedKey(UpperBody));
    hands           = Equipment.Get(Hands, getEquippedKey(Hands));
    lowerBody       = Equipment.Get(LowerBody, getEquippedKey(LowerBody));
    feet            = Equipment.Get(Feet, getEquippedKey(Feet));
    primaryWeapon   = Equipment.Get(Weapon, getEquippedKey(PrimaryWeapon));
    secondaryWeapon = Equipment.Get(Weapon, getEquippedKey(SecondaryWeapon));
  }


  public function serialise(saveData: SaveData): Void {
    saveData.health             = health;
    saveData.stamina            = stamina;
    saveData.headKey            = Helpers.EnumToInt(getEquippedKey(Head));
    saveData.upperBodyKey       = Helpers.EnumToInt(getEquippedKey(UpperBody));
    saveData.handsKey           = Helpers.EnumToInt(getEquippedKey(Hands));
    saveData.lowerBodyKey       = Helpers.EnumToInt(getEquippedKey(LowerBody));
    saveData.feetKey            = Helpers.EnumToInt(getEquippedKey(Feet));
    saveData.primaryWeaponKey   = Helpers.EnumToInt(getEquippedKey(PrimaryWeapon));
    saveData.secondaryWeaponKey = Helpers.EnumToInt(getEquippedKey(SecondaryWeapon));
  }

  public function deserialise(saveData: SaveData): Void {
    health                                = saveData.health;
    stamina                               = saveData.stamina;
    equippedKeys[EquipmentSlot.Head]      = Helpers.IntToEnum(GameEquipmentHead, saveData.headKey);
    equippedKeys[EquipmentSlot.UpperBody] = Helpers.IntToEnum(GameEquipmentUpperBody, saveData.upperBodyKey);
    equippedKeys[EquipmentSlot.Hands]     = Helpers.IntToEnum(GameEquipmentHands, saveData.handsKey);
    equippedKeys[EquipmentSlot.LowerBody] = Helpers.IntToEnum(GameEquipmentLowerBody, saveData.lowerBodyKey);
    equippedKeys[EquipmentSlot.Feet]      = Helpers.IntToEnum(GameEquipmentFeet, saveData.feetKey);
    equippedKeys[PrimaryWeapon]           = Helpers.IntToEnum(GameEquipmentWeapon, saveData.primaryWeaponKey);
    equippedKeys[SecondaryWeapon]         = Helpers.IntToEnum(GameEquipmentWeapon, saveData.secondaryWeaponKey);
    updateEquipped();
  }
}
