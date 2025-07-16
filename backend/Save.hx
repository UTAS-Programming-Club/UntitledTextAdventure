package backend;

import backend.saving.Base85;
using backend.saving.ByteHelpers;
import backend.Game;
import haxe.io.Bytes;

  // If adding compression, version must be prepended after compression is done but before encoding.
final SaveVersion: Int = 1; // 2 Bytes, 65535 is reserved
final SaveDataSize: Int = Math.ceil(1 * 2 + 2 * 7/8 + 2 * 5/8 + 7 * 1);

// TODO: Store screen state
// TODO: Generate via a macro
class SaveData {
  public var health:  Int = -1; // 7 Bits, [1, 100]
  public var stamina: Int = -1; // 7 Bits, [0, 100]

  public var roomX: Int = -1; // 5 bits, [0, 15]
  public var roomY: Int = -1; // 5 bits, [0, 15]

  // Original plan says 12 sets for the body but ideas page says 9
  // Listed bounds are currently wrong as GameEquipment isn't complete
  public var headIdx:            Int = -1; // 4 Bits, [00, 11]
  public var upperBodyIdx:       Int = -1; // 4 Bits, [12, 23]
  public var handsIdx:           Int = -1; // 4 Bits, [24, 35]
  public var lowerBodyIdx:       Int = -1; // 4 Bits, [36, 47]
  public var feetIdx:            Int = -1; // 4 Bits, [48, 59]
  public var primaryWeaponIdx:   Int = -1; // 4 Bits, [60, 75]
  public var secondaryWeaponIdx: Int = -1; // 4 Bits, [60, 75]

  public function new(state: Game) {
    if (state.campaign.rooms.length > 15) {
      throw ': Map to big to save';
    }
  }

  // min and max are inclusive bounds
  public function checkVal(val: Int, min: Int, max: Int): Bool {
    return val >= min && val <= max;
  }

  public function serialize(): Bytes {
   if (health == -1 ||
       stamina == -1 ||
       roomX == -1 ||
       roomY == -1 ||
       headIdx == -1 ||
       upperBodyIdx == -1 ||
       handsIdx == -1 ||
       lowerBodyIdx == -1 ||
       feetIdx == -1 ||
       primaryWeaponIdx == -1 ||
       secondaryWeaponIdx == -1) {
      throw ': Unable to save game, some fields have not been set';
    }

    final buffer: Bytes = Bytes.alloc(SaveDataSize);
    buffer.fill(0, SaveDataSize, 0);

    var offset: Int = 0;
    offset = buffer.setBitInt(offset, SaveVersion, 16);
    offset = buffer.setBitInt(offset, health, 7);
    offset = buffer.setBitInt(offset, stamina, 7);
    offset = buffer.setBitInt(offset, roomX, 5);
    offset = buffer.setBitInt(offset, roomY, 5);
    offset = buffer.setBitInt(offset, headIdx, 8);
    offset = buffer.setBitInt(offset, upperBodyIdx, 8);
    offset = buffer.setBitInt(offset, handsIdx, 8);
    offset = buffer.setBitInt(offset, lowerBodyIdx, 8);
    offset = buffer.setBitInt(offset, feetIdx, 8);
    offset = buffer.setBitInt(offset, primaryWeaponIdx, 8);
    offset = buffer.setBitInt(offset, secondaryWeaponIdx, 8);
    return buffer;
  }

  // Assumes data.length == SaveDataSize
  public function deserialise(state: Game, bytes: Bytes): Bool {
    var offset: Int = 0;
    final version: Int = bytes.getBitInt(offset, 16); offset += 16;
    if (version != SaveVersion) {
      throw ': Unexpected save version $version';
    }

    var valid: Bool = true;

    health = bytes.getBitInt(offset, 7);
    valid = valid && checkVal(health, 1, 100);
    offset += 7;

    stamina = bytes.getBitInt(offset, 7);
    valid = valid && checkVal(stamina, 0, 100);
    offset += 7;

    roomX = bytes.getBitInt(offset, 5);
    valid = valid && checkVal(roomX, 0, state.campaign.rooms.length);
    offset += 5;

    roomY = bytes.getBitInt(offset, 5);
    valid = valid && checkVal(roomY, 0, state.campaign.rooms.length);
    offset += 5;

    final equipmentMax: Int = state.campaign.equipmentOrder.length - 1;

    headIdx = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(headIdx, 0, equipmentMax);
    offset += 8;

    upperBodyIdx = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(upperBodyIdx, 0, equipmentMax);
    offset += 8;

    handsIdx = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(handsIdx, 0, equipmentMax);
    offset += 8;

    lowerBodyIdx = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(lowerBodyIdx, 0, equipmentMax);
    offset += 8;

    feetIdx = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(feetIdx, 0, equipmentMax);
    offset += 8;

    primaryWeaponIdx = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(primaryWeaponIdx, 0, equipmentMax);
    offset += 8;

    secondaryWeaponIdx = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(secondaryWeaponIdx, 0, equipmentMax);

    return valid;
  }
}

function Save(state: Game): UnicodeString {
  final saveData = new SaveData(state);
  state.player.serialise(state.campaign, saveData);

  final bytes: Bytes = saveData.serialize();
  return Base85.encode(bytes);
}

function Load(state: Game, str: UnicodeString): Bool {
  final bytes: Bytes = Base85.decode(str, SaveDataSize);
  if (bytes.length != SaveDataSize) {
    return false;
  }

  final saveData = new SaveData(state);
  if (!saveData.deserialise(state, bytes)) {
    return false;
  }

  state.player.deserialise(state.campaign, saveData);
  return true;
}
