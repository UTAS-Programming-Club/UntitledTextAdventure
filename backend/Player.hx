package backend;

import backend.saving.Helpers;
import backend.Campaign;
import backend.Equipment;
import backend.GameInfo;
import backend.Save;

class Player {
  public var health(default, null): Int = 0;  // Must be in [0, 100]
  public var stamina(default, null): Int = 0; // Must be in [0, 100]

  private var headKey: GameEquipmentHead;
  public var head(default, null): Equipment;

  private var upperBodyKey: GameEquipmentUpperBody;
  public var upperBody(default, null): Equipment;

  private var handsKey: GameEquipmentHands;
  public var hands(default, null): Equipment;

  private var lowerBodyKey: GameEquipmentLowerBody;
  public var lowerBody(default, null): Equipment;

  private var feetKey: GameEquipmentFeet;
  public var feet(default, null): Equipment;

  private var primaryWeaponKey: GameEquipmentWeapon;
  public var primaryWeapon(default, null): Equipment;

  private var secondaryWeaponKey: GameEquipmentWeapon;
  public var secondaryWeapon(default, null): Equipment;

  public function new(campaign: Campaign) {
    headKey = campaign.initialHead;
    head = Equipment.Get(Head, headKey);

    upperBodyKey = campaign.initialUpperBody;
    upperBody = Equipment.Get(UpperBody, upperBodyKey);

    handsKey = campaign.initialHands;
    hands = Equipment.Get(Hands, handsKey);

    lowerBodyKey = campaign.initialLowerBody;
    lowerBody = Equipment.Get(LowerBody, lowerBodyKey);

    feetKey = campaign.initialFeet;
    feet = Equipment.Get(Feet, feetKey);

    primaryWeaponKey = campaign.initialPrimaryWeapon;
    primaryWeapon = Equipment.Get(Weapon, primaryWeaponKey);

    secondaryWeaponKey = campaign.initialSecondaryWeapon;
    secondaryWeapon = Equipment.Get(Weapon, secondaryWeaponKey);
  }

  public function Reset(campaign: Campaign): Void {
    health = 100;
    stamina = 100;

    headKey = campaign.initialHead;
    upperBodyKey = campaign.initialUpperBody;
    handsKey = campaign.initialHands;
    lowerBodyKey = campaign.initialLowerBody;
    feetKey = campaign.initialFeet;
    primaryWeaponKey = campaign.initialPrimaryWeapon;
    secondaryWeaponKey = campaign.initialSecondaryWeapon;
    updateEquipment();
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


  private function updateEquipment() {
    head            = Equipment.Get(Head, headKey);
    upperBody       = Equipment.Get(UpperBody, upperBodyKey);
    hands           = Equipment.Get(Hands, handsKey);
    lowerBody       = Equipment.Get(LowerBody, lowerBodyKey);
    feet            = Equipment.Get(Feet, feetKey);
    primaryWeapon   = Equipment.Get(Weapon, primaryWeaponKey);
    secondaryWeapon = Equipment.Get(Weapon, secondaryWeaponKey);
  }


  public function serialise(saveData: SaveData): Void {
    saveData.health             = health;
    saveData.stamina            = stamina;
    saveData.headKey            = Helpers.EnumToInt(headKey);
    saveData.upperBodyKey       = Helpers.EnumToInt(upperBodyKey);
    saveData.handsKey           = Helpers.EnumToInt(handsKey);
    saveData.lowerBodyKey       = Helpers.EnumToInt(lowerBodyKey);
    saveData.feetKey            = Helpers.EnumToInt(feetKey);
    saveData.primaryWeaponKey   = Helpers.EnumToInt(primaryWeaponKey);
    saveData.secondaryWeaponKey = Helpers.EnumToInt(secondaryWeaponKey);
  }

  public function deserialise(saveData: SaveData): Void {
    health             = saveData.health;
    stamina            = saveData.stamina;
    headKey            = Helpers.IntToEnum(GameEquipmentHead, saveData.headKey);
    upperBodyKey       = Helpers.IntToEnum(GameEquipmentUpperBody, saveData.upperBodyKey);
    handsKey           = Helpers.IntToEnum(GameEquipmentHands, saveData.handsKey);
    lowerBodyKey       = Helpers.IntToEnum(GameEquipmentLowerBody, saveData.lowerBodyKey);
    feetKey            = Helpers.IntToEnum(GameEquipmentFeet, saveData.feetKey);
    primaryWeaponKey   = Helpers.IntToEnum(GameEquipmentWeapon, saveData.primaryWeaponKey);
    secondaryWeaponKey = Helpers.IntToEnum(GameEquipmentWeapon, saveData.secondaryWeaponKey);
    updateEquipment();
  }
}
