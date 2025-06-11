package backend;

import backend.compression.Base85;
using backend.compression.ByteHelpers;
import backend.Game;
import backend.GameInfo;
import haxe.io.Bytes;

  // If adding compression, version must be prepended after compression is done but before encoding.
final SaveVersion: Int = 1; // 2 Bytes, 65535 is reserved
final SaveDataSize: Int = Math.ceil(1 * 2 + 2 * 7/8 + 7 * 1);

// TODO: Store screen state
class SaveData {
  public var health: Int = 0;  // 7 Bits, [1, 100]
  public var stamina: Int = 0; // 7 Bits, [1, 100]

  // Original plan says 12 sets for the body but ideas page says 9
  // Listed bounds are currently wrong as GameEquipment isn't complete
  public var headKey: GameEquipment = 0;            // 4 Bits, [00, 11]
  public var upperBodyKey: GameEquipment = 0;       // 4 Bits, [12, 23]
  public var handsKey: GameEquipment = 0;           // 4 Bits, [24, 35]
  public var lowerBodyKey: GameEquipment = 0;       // 4 Bits, [36, 47]
  public var feetKey: GameEquipment = 0;            // 4 Bits, [48, 59]
  public var primaryWeaponKey: GameEquipment = 0;   // 4 Bits, [60, 75]
  public var secondaryWeaponKey: GameEquipment = 0; // 4 Bits, [60, 75]

  public function new() {
  }

  public function serialize(): Bytes {
    final buffer: Bytes = Bytes.alloc(SaveDataSize);
    var offset: Int = 0;
    offset = buffer.setBitInt(offset, SaveVersion, 16);
    offset = buffer.setBitInt(offset, health, 7);
    offset = buffer.setBitInt(offset, stamina, 7);
    offset = buffer.setBitInt(offset, cast(headKey, Int), 8);
    offset = buffer.setBitInt(offset, cast(upperBodyKey, Int), 8);
    offset = buffer.setBitInt(offset, cast(handsKey, Int), 8);
    offset = buffer.setBitInt(offset, cast(lowerBodyKey, Int), 8);
    offset = buffer.setBitInt(offset, cast(feetKey, Int), 8);
    offset = buffer.setBitInt(offset, cast(primaryWeaponKey, Int), 8);
    offset = buffer.setBitInt(offset, cast(secondaryWeaponKey, Int), 8);
    return buffer;
  }

  // Assumes data.length == SaveDataSize
  public function deserialise(bytes: Bytes) {
    var offset: Int = 0;
    final version: Int = bytes.getBitInt(offset, 16); offset += 16;
    if (version != SaveVersion) {
      throw 'Unexpected save version $version';
    }

    health =             bytes.getBitInt(offset, 7); offset += 7;
    stamina =            bytes.getBitInt(offset, 7); offset += 7;
    headKey =            bytes.getBitInt(offset, 8); offset += 8;
    upperBodyKey =       bytes.getBitInt(offset, 8); offset += 8;
    handsKey =           bytes.getBitInt(offset, 8); offset += 8;
    lowerBodyKey =       bytes.getBitInt(offset, 8); offset += 8;
    feetKey =            bytes.getBitInt(offset, 8); offset += 8;
    primaryWeaponKey =   bytes.getBitInt(offset, 8); offset += 8;
    secondaryWeaponKey = bytes.getBitInt(offset, 8);
  }
}

function Save(state: Game): UnicodeString {
  final saveData = new SaveData();
  state.player.serialise(saveData);

  final bytes: Bytes = saveData.serialize();
  return Base85.encode(bytes);
}

// TODO: Modify game state
function Load(state: Game, str: UnicodeString): Bool {
  final bytes: Bytes = Base85.decode(str, SaveDataSize);
  if (bytes.length != SaveDataSize) {
    return false;
  }

  final saveData = new SaveData();
  saveData.deserialise(bytes);
  state.player.deserialise(saveData);
  return true;
}
