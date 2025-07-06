package backend;

import backend.Campaign;
import backend.Equipment;
import backend.GameInfo;

// Only import backend.Player from backend.Game

class Player {
  public var health(default, null): Int = 0;  // Must be in [0, 100]
  public var stamina(default, null): Int = 0; // Must be in [0, 100]

  // For some reason ++ and possibly -- works despite disabling public assignment
  public var x(default, null): Int = 0; // Must be in [0, campaign.rooms.length)
  public var y(default, null): Int = 0; // Must be in [0, campaign.rooms.length)

  // Only add items via unlockItem
  private var unlockedItems: Map<EquipmentType, Array<GameEquipment>> = [];
  // TODO: Switch to "equippedItems: Map<EquipmentType, GameEquipment>"?
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

  public function reset(campaign: Campaign): Void {
    health = 100;
    stamina = 100;

    changeRoom(campaign, campaign.initialRoomX, campaign.initialRoomY);

    unlockedItems = [];
    unlockItem(campaign, campaign.initialHead);
    unlockItem(campaign, campaign.initialUpperBody);
    unlockItem(campaign, campaign.initialHands);
    unlockItem(campaign, campaign.initialLowerBody);
    unlockItem(campaign, campaign.initialFeet);
    unlockItem(campaign, campaign.initialPrimaryWeapon);
    unlockItem(campaign, campaign.initialSecondaryWeapon);

    equipItem(campaign.initialHead, Head);
    equipItem(campaign.initialUpperBody, UpperBody);
    equipItem(campaign.initialHands, Hands);
    equipItem(campaign.initialLowerBody, LowerBody);
    equipItem(campaign.initialFeet, Feet);
    equipItem(campaign.initialPrimaryWeapon, PrimaryWeapon);
    equipItem(campaign.initialSecondaryWeapon, SecondaryWeapon);
  }


  public function unlockItem(campaign: Campaign, item: GameEquipment): Void {
#if debuggame
    // Assumes extension equipment declarations and equipmentOrder are valid, both are handled in Game.hx
    if (!campaign.equipmentOrder.contains(item)) {
      throw ': Invalid equipment ${item.type}:${item.name}';
    }
#end

    final unlockedSlotItems = unlockedItems[item.type];
    if (unlockedSlotItems == null) {
      unlockedItems[item.type] = [item];
    } else {
      unlockedSlotItems.push(item);
    }
  }

  function equipItem(item: GameEquipment, slot: EquipmentSlot): Void {
#if debuggame
    final unlockedSlotItems = unlockedItems[item.type];
    if (unlockedSlotItems == null || !unlockedSlotItems.contains(item)) {
      throw ': Attempted to equip locked item ${item.type}:${item.name}';
    }
#end

    if (Equipment.getType(slot) != item.type) {
      throw ': Mismatch between item type ${item.type} and requested slot $slot';
    }

    switch (slot) {
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

  public function cycleItemSlot(slot: EquipmentSlot): Void {
    final unlockedSlotItems = unlockedItems[Equipment.getType(slot)];
    if (unlockedSlotItems == null) {
      // TODO: Throw instead?
      return;
    }

    final currentItem: GameEquipment = switch (slot) {
      case Head:
        head;
      case UpperBody:
        upperBody;
      case Hands:
        hands;
      case LowerBody:
        lowerBody;
      case Feet:
        feet;
      case PrimaryWeapon:
        primaryWeapon;
      case SecondaryWeapon:
        secondaryWeapon;
    }

    var foundCurrentItem: Bool = false;
    var cycledItem: Bool = false;
    for (item in unlockedSlotItems) {
      if (!foundCurrentItem) {
        foundCurrentItem = currentItem == item;
        continue;
      }

      switch (slot) {
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

      cycledItem = true;
      break;
    }

    if (!cycledItem) {
      switch (slot) {
        case Head:
          head = unlockedSlotItems[0];
        case UpperBody:
          upperBody = unlockedSlotItems[0];
        case Hands:
          hands = unlockedSlotItems[0];
        case LowerBody:
          lowerBody = unlockedSlotItems[0];
        case Feet:
          feet = unlockedSlotItems[0];
        case PrimaryWeapon:
          primaryWeapon = unlockedSlotItems[0];
        case SecondaryWeapon:
          secondaryWeapon = unlockedSlotItems[0];
      }
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
