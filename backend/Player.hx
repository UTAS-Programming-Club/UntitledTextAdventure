package backend;

import backend.Campaign;
// import backend.Equipment;
import backend.GameInfo;

// Only import backend.Player from backend.Game

class Player {
  public var health(default, null): Int = 0;  // Must be in [0, 100]
  public var stamina(default, null): Int = 0; // Must be in [0, 100]

  // For some reason ++ and possibly -- works despite disabling public assignment
  public var x(default, null): Int = 0; // Must be in [0, campaign.rooms.length)
  public var y(default, null): Int = 0; // Must be in [0, campaign.rooms.length)

  public var head(default, null): GameEquipment;
  public var upperBody(default, null): GameEquipment;
  public var hands(default, null): GameEquipment;
  public var lowerBody(default, null): GameEquipment;
  public var feet(default, null): GameEquipment;
  public var primaryWeapon(default, null): GameEquipment;
  public var secondaryWeapon(default, null): GameEquipment;

  public function new(campaign: Campaign) {
    head = campaign.initialHead;
    upperBody = campaign.initialUpperBody;
    hands = campaign.initialHands;
    lowerBody = campaign.initialLowerBody;
    feet = campaign.initialFeet;
    primaryWeapon = campaign.initialPrimaryWeapon;
    secondaryWeapon = campaign.initialSecondaryWeapon;
  }

  public function Reset(campaign: Campaign): Void {
    health = 100;
    stamina = 100;

    changeRoom(campaign, campaign.initialRoomX, campaign.initialRoomY);

    equipItem(campaign, campaign.initialHead);
    equipItem(campaign, campaign.initialUpperBody);
    equipItem(campaign, campaign.initialHands);
    equipItem(campaign, campaign.initialLowerBody);
    equipItem(campaign, campaign.initialFeet);
    equipItem(campaign, campaign.initialPrimaryWeapon);
    equipItem(campaign, campaign.initialSecondaryWeapon);
  }


  public function equipItem(campaign: Campaign, item: GameEquipment): Void {
#if debuggame
    // Assumes extension equipment declarations and equipmentOrder are valid, both are handled but Game.hx
    if (!campaign.equipmentOrder.contains(item)) {
      throw ': Invalid equipment ${item.type}:${item.name}';
    }
#end

    switch (item.type) {
      case Head:
        head = item;
      case UpperBody:
        upperBody = item;
      case Hands:
        hands = item;
      case LowerBody:
        lowerBody = item;
      case Feet:
        feet = item;
      case PrimaryWeapon:
        primaryWeapon = item;
      case SecondaryWeapon:
        secondaryWeapon = item;
    }
  }


  // TODO: End game on 0 health
  // change is clamped to [0, 100]
  public function modifyHealth(change: Int): Void {
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
