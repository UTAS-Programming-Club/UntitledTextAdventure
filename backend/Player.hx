package backend;

import backend.Equipment;
import backend.Game;
import backend.GameInfo;

// Only import from backend.Game

class Player {
  public var health(default, null): Int = 0;  // Must be in [0, 100]
  public var stamina(default, null): Int = 0; // Must be in [0, 100]
  // TODO: Set this based on equipment stats
  public var agility(default, null): Int = 20; // Must be in [0, 100]

  // TODO: Set these based on equipment stats
  public final physicalDefence: Int = 2; // Must be in [0, 100]
  public final magicalDefence: Int = 3;  // Must be in [0, 100]

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

  public var hasMap(default, null): Bool = true;

  public function new(state: Game) {
    head = state.campaign.initialHead;
    upperBody = state.campaign.initialUpperBody;
    hands = state.campaign.initialHands;
    lowerBody = state.campaign.initialLowerBody;
    feet = state.campaign.initialFeet;
    primaryWeapon = state.campaign.initialPrimaryWeapon;
    secondaryWeapon = state.campaign.initialSecondaryWeapon;
  }

  public function reset(state: Game): Void {
    health = 100;
    stamina = 100;

    changeRoom(state, state.campaign.initialRoomX, state.campaign.initialRoomY);

    unlockedItems = [];
    unlockItem(state, state.campaign.initialHead);
    unlockItem(state, state.campaign.initialUpperBody);
    unlockItem(state, state.campaign.initialHands);
    unlockItem(state, state.campaign.initialLowerBody);
    unlockItem(state, state.campaign.initialFeet);
    unlockItem(state, state.campaign.initialPrimaryWeapon);
    unlockItem(state, state.campaign.initialSecondaryWeapon);

    equipItem(state.campaign.initialHead, Head);
    equipItem(state.campaign.initialUpperBody, UpperBody);
    equipItem(state.campaign.initialHands, Hands);
    equipItem(state.campaign.initialLowerBody, LowerBody);
    equipItem(state.campaign.initialFeet, Feet);
    equipItem(state.campaign.initialPrimaryWeapon, PrimaryWeapon);
    equipItem(state.campaign.initialSecondaryWeapon, SecondaryWeapon);
  }


  public function unlockItem(state: Game, item: GameEquipment): Void {
#if debuggame
    // Assumes extension equipment declarations and equipmentOrder are valid, both are handled in Game.hx
    if (!state.campaign.equipmentOrder.contains(item)) {
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


  // x and y must be in [0, state.campaign.rooms.length)
  public function changeRoom(state: Game, x: Int, y: Int): Void {
    if (x < 0 || y < 0 || x >= state.campaign.rooms.length || y >= state.campaign.rooms.length) {
      throw 'Room $x, $y is out of bounds';
    }

    this.x = x;
    this.y = y;
  }
}
