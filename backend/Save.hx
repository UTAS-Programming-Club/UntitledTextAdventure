package backend;

import backend.saving.Base85;
using backend.saving.Helpers;
import backend.Game;
import backend.GameInfo;
import haxe.io.Bytes;

  // If adding compression, version must be prepended after compression is done but before encoding.
final SaveVersion: Int = 1; // 2 Bytes, 65535 is reserved
final SaveDataSize: Int = Math.ceil(1 * 2 + 2 * 7/8 + 7 * 1);

// TODO: Store screen state
// TODO: Generate via a macro
class SaveData {
  public var health:  Int = -1; // 7 Bits, [1, 100]
  public var stamina: Int = -1; // 7 Bits, [1, 100]

  // Original plan says 12 sets for the body but ideas page says 9
  // Listed bounds are currently wrong as GameEquipment isn't complete
  public var headKey:            Int = -1; // 4 Bits, [00, 11]
  public var upperBodyKey:       Int = -1; // 4 Bits, [12, 23]
  public var handsKey:           Int = -1; // 4 Bits, [24, 35]
  public var lowerBodyKey:       Int = -1; // 4 Bits, [36, 47]
  public var feetKey:            Int = -1; // 4 Bits, [48, 59]
  public var primaryWeaponKey:   Int = -1; // 4 Bits, [60, 75]
  public var secondaryWeaponKey: Int = -1; // 4 Bits, [60, 75]

  public function new() {
  }

  // min and max are inclusive bounds
  public function checkVal(val: Int, min: Int, max: Int): Bool {
    return val >= min && val <= max;
  }

  public function serialize(): Bytes {
   if (health == -1 ||
       stamina == -1 ||
       headKey == -1 ||
       upperBodyKey == -1 ||
       handsKey == -1 ||
       lowerBodyKey == -1 ||
       feetKey == -1 ||
       primaryWeaponKey == -1 ||
       secondaryWeaponKey == -1) {
      throw 'Unable to save game, some fields have not been set.';
    }

    final buffer: Bytes = Bytes.alloc(SaveDataSize);
    buffer.fill(0, SaveDataSize, 0);

    var offset: Int = 0;
    offset = buffer.setBitInt(offset, SaveVersion, 16);
    offset = buffer.setBitInt(offset, health, 7);
    offset = buffer.setBitInt(offset, stamina, 7);
    offset = buffer.setBitInt(offset, headKey, 8);
    offset = buffer.setBitInt(offset, upperBodyKey, 8);
    offset = buffer.setBitInt(offset, handsKey, 8);
    offset = buffer.setBitInt(offset, lowerBodyKey, 8);
    offset = buffer.setBitInt(offset, feetKey, 8);
    offset = buffer.setBitInt(offset, primaryWeaponKey, 8);
    offset = buffer.setBitInt(offset, secondaryWeaponKey, 8);
    return buffer;
  }

  // Assumes data.length == SaveDataSize
  public function deserialise(bytes: Bytes): Bool {
    var offset: Int = 0;
    final version: Int = bytes.getBitInt(offset, 16); offset += 16;
    if (version != SaveVersion) {
      throw 'Unexpected save version $version';
    }

    var valid: Bool = true;

    health = bytes.getBitInt(offset, 7);
    valid = valid && checkVal(health, 1, 100);
    offset += 7;

    stamina = bytes.getBitInt(offset, 7);
    valid = valid && checkVal(stamina, 1, 100);
    offset += 7;

    final equipmentMax: Int = Type.getEnumConstructs(GameEquipment).length - 1;

    headKey = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(headKey, 0, equipmentMax);
    offset += 8;

    upperBodyKey = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(upperBodyKey, 0, equipmentMax);
    offset += 8;

    handsKey = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(handsKey, 0, equipmentMax);
    offset += 8;

    lowerBodyKey = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(lowerBodyKey, 0, equipmentMax);
    offset += 8;

    feetKey = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(feetKey, 0, equipmentMax);
    offset += 8;

    primaryWeaponKey = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(primaryWeaponKey, 0, equipmentMax);
    offset += 8;

    secondaryWeaponKey = bytes.getBitInt(offset, 8);
    valid = valid && checkVal(secondaryWeaponKey, 0, equipmentMax);

    return valid;
  }
}

function Save(state: Game): UnicodeString {
  final saveData = new SaveData();
  state.player.serialise(saveData);

  final bytes: Bytes = saveData.serialize();
  return Base85.encode(bytes);
}

function Load(state: Game, str: UnicodeString): Bool {
  final bytes: Bytes = Base85.decode(str, SaveDataSize);
  if (bytes.length != SaveDataSize) {
    return false;
  }

  final saveData = new SaveData();
  if (!saveData.deserialise(bytes)) {
    return false;
  }

  state.player.deserialise(saveData);
  return true;
}
