package backend;

import backend.compression.Base85;
using backend.compression.ByteHelpers;
import backend.Game;
import backend.GameInfo;
import haxe.io.Bytes;
import haxe.io.BytesBuffer;

final SaveVersion: Int = 1;
final SaveDataSize: Int = Std.int(1 * 2 + 2 * 7/8 + 8 * 1);


// TODO: Store screen state
// TODO: Avoid data being nullable
// NOTE: if the constructor doesn't throw then no fields are null.
class SaveData {
  // NOTE: If adding compression, version but be prepended after compression is done but before encoding.
  private final version: Int = SaveVersion; // 2 Bytes, 65535 is reserved

  public var health: Null<Int>;  // 7 Bits, [1, 100]
  public var stamina: Null<Int>; // 7 Bits, [1, 100]

  // Original plan says 12 sets for the body but ideas page says 9
  // Listed bounds are currently wrong as GameEquipment isn't complete
  public var headKey: Null<GameEquipment>;            // 4 Bits, [00, 11]
  public var upperBodyKey: Null<GameEquipment>;       // 4 Bits, [12, 23]
  public var handsKey: Null<GameEquipment>;           // 4 Bits, [24, 35]
  public var lowerBodyKey: Null<GameEquipment>;       // 4 Bits, [36, 47]
  public var feetKey: Null<GameEquipment>;            // 4 Bits, [48, 59]
  public var primaryWeaponKey: Null<GameEquipment>;   // 4 Bits, [60, 75]
  public var secondaryWeaponKey: Null<GameEquipment>; // 4 Bits, [60, 75]

  public function new(state: Game) {
    state.player.serialise(this);

    if (health == null ||
        stamina == null ||
        headKey == null ||
        upperBodyKey == null ||
        handsKey == null ||
        lowerBodyKey == null ||
        feetKey == null ||
        primaryWeaponKey == null ||
        secondaryWeaponKey == null) {
      throw 'Unable to save game';
    }
  }

  public function serialize(): Bytes {
    final buffer: Bytes = Bytes.alloc(SaveDataSize);
    var offset: Int = 0;
    offset = buffer.setBitInt(offset, cast health, 7);
    offset = buffer.setBitInt(offset, cast stamina, 7);
    offset = buffer.setBitInt(offset, cast headKey, 8);
    offset = buffer.setBitInt(offset, cast upperBodyKey, 8);
    offset = buffer.setBitInt(offset, cast handsKey, 8);
    offset = buffer.setBitInt(offset, cast lowerBodyKey, 8);
    offset = buffer.setBitInt(offset, cast feetKey, 8);
    offset = buffer.setBitInt(offset, cast primaryWeaponKey, 8);
    offset = buffer.setBitInt(offset, cast secondaryWeaponKey, 8);
    return buffer;
  }

  // Assumes data.length == SaveDataSize
  public function unserialize(bytes: Bytes) {
    // version = ctx.getByte() << 0x8 + ctx.getByte();
  }
}

function Save(state: Game): UnicodeString {
  final saveData = new SaveData(state);
  final bytes: Bytes = saveData.serialize();
  return Base85.encode(bytes);
}

function Load(): Void {
  throw "Loading not implemented";
}
