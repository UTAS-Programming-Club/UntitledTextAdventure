package backend;

import backend.saving.Helpers;
import backend.Campaign;
import backend.Equipment;
import backend.GameInfo;
import backend.Save;

class Player {
  public var health(default, null): Int = 0;  // Must be in [0, 100]
  public var stamina(default, null): Int = 0; // Must be in [0, 100]

  private var headKey: GameEquipment;
  public var head(default, null): Equipment;

  private var upperBodyKey: GameEquipment;
  public var upperBody(default, null): Equipment;

  private var handsKey: GameEquipment;
  public var hands(default, null): Equipment;

  private var lowerBodyKey: GameEquipment;
  public var lowerBody(default, null): Equipment;

  private var feetKey: GameEquipment;
  public var feet(default, null): Equipment;

  private var primaryWeaponKey: GameEquipment;
  public var primaryWeapon(default, null): Equipment;

  private var secondaryWeaponKey: GameEquipment;
  public var secondaryWeapon(default, null): Equipment;

  public function new(campaign: Campaign) {
    headKey = campaign.initialHead;
    head = Equipment.Get(campaign.initialHead);

    upperBodyKey = campaign.initialUpperBody;
    upperBody = Equipment.Get(campaign.initialUpperBody);

    handsKey = campaign.initialHands;
    hands = Equipment.Get(campaign.initialHands);

    lowerBodyKey = campaign.initialLowerBody;
    lowerBody = Equipment.Get(campaign.initialLowerBody);

    feetKey = campaign.initialFeet;
    feet = Equipment.Get(campaign.initialFeet);

    primaryWeaponKey = campaign.initialPrimaryWeapon;
    primaryWeapon = Equipment.Get(campaign.initialPrimaryWeapon);

    secondaryWeaponKey = campaign.initialSecondaryWeapon;
    secondaryWeapon = Equipment.Get(campaign.initialSecondaryWeapon);
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
    head            = Equipment.Get(headKey);
    upperBody       = Equipment.Get(upperBodyKey);
    hands           = Equipment.Get(handsKey);
    lowerBody       = Equipment.Get(lowerBodyKey);
    feet            = Equipment.Get(feetKey);
    primaryWeapon   = Equipment.Get(primaryWeaponKey);
    secondaryWeapon = Equipment.Get(secondaryWeaponKey);
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
    headKey            = Helpers.IntToEnum(GameEquipment, saveData.headKey);
    upperBodyKey       = Helpers.IntToEnum(GameEquipment, saveData.upperBodyKey);
    handsKey           = Helpers.IntToEnum(GameEquipment, saveData.handsKey);
    lowerBodyKey       = Helpers.IntToEnum(GameEquipment, saveData.lowerBodyKey);
    feetKey            = Helpers.IntToEnum(GameEquipment, saveData.feetKey);
    primaryWeaponKey   = Helpers.IntToEnum(GameEquipment, saveData.primaryWeaponKey);
    secondaryWeaponKey = Helpers.IntToEnum(GameEquipment, saveData.secondaryWeaponKey);
    updateEquipment();
  }
}
