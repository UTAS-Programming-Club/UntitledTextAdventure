package backend;

import backend.Campaign;
// import backend.Equipment;
// import backend.GameInfo;

// Only import backend.Player from backend.Game

class Player {
  public var health(default, null): Int = 0;  // Must be in [0, 100]
  public var stamina(default, null): Int = 0; // Must be in [0, 100]

  // For some reason ++ and possibly -- works despite disabling public assignment
  public var x(default, null): Int = 0; // Must be in [0, campaign.rooms.length)
  public var y(default, null): Int = 0; // Must be in [0, campaign.rooms.length)

  // public var head(default, null): Equipment;
  // public var upperBody(default, null): Equipment;
  // public var hands(default, null): Equipment;
  // public var lowerBody(default, null): Equipment;
  // public var feet(default, null): Equipment;
  // public var primaryWeapon(default, null): Equipment;
  // public var secondaryWeapon(default, null): Equipment;

  public function new(campaign: Campaign) {
    // head = Equipment.Get(campaign.initialHead);
    // upperBody = Equipment.Get(campaign.initialUpperBody);
    // hands = Equipment.Get(campaign.initialHands);
    // lowerBody = Equipment.Get(campaign.initialLowerBody);
    // feet = Equipment.Get(campaign.initialFeet);
    // primaryWeapon = Equipment.Get(campaign.initialPrimaryWeapon);
    // secondaryWeapon = Equipment.Get(campaign.initialSecondaryWeapon);
  }

  public function Reset(campaign: Campaign): Void {
    health = 100;
    stamina = 100;

    x = campaign.initialRoomY;
    y = campaign.initialRoomY;

    // head = Equipment.Get(campaign.initialHead);
    // upperBody = Equipment.Get(campaign.initialUpperBody);
    // hands = Equipment.Get(campaign.initialHands);
    // lowerBody = Equipment.Get(campaign.initialLowerBody);
    // feet = Equipment.Get(campaign.initialFeet);
    // primaryWeapon = Equipment.Get(campaign.initialPrimaryWeapon);
    // secondaryWeapon = Equipment.Get(campaign.initialSecondaryWeapon);
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

  // x and y must be in [0, campaign.rooms.length)
  public function changeRoom(campaign: Campaign, x: Int, y: Int): Void {
    if (x < 0 || y < 0 || x >= campaign.rooms.length || y >= campaign.rooms.length) {
      throw 'Room $x, $y is out of bounds';
    }

    this.x = x;
    this.y = y;
  }
}
