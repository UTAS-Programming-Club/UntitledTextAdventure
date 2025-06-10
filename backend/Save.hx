package backend;

import backend.compression.Base85;
using backend.compression.ByteHelpers;
import backend.Game;
import haxe.io.Bytes;

  // If adding compression, version must be prepended after compression is done but before encoding.
final SaveVersion: Int = 1; // 2 Bytes, 65535 is reserved
final SaveDataSize: Int = Std.int(1 * 2 + 2 * 7/8 + 8 * 1);

// TODO: Store screen state
// TODO: Avoid data being nullable
// NOTE: if the constructor doesn't throw then no fields are null.
class SaveData {
  public var health: Null<Int>;  // 7 Bits, [1, 100]
  public var stamina: Null<Int>; // 7 Bits, [1, 100]

  // Original plan says 12 sets for the body but ideas page says 9
  // Listed bounds are currently wrong as GameEquipment isn't complete
  public var headIdx: Null<Int>;            // 4 Bits, [00, 11]
  public var upperBodyIdx: Null<Int>;       // 4 Bits, [12, 23]
  public var handsIdx: Null<Int>;           // 4 Bits, [24, 35]
  public var lowerBodyIdx: Null<Int>;       // 4 Bits, [36, 47]
  public var feetIdx: Null<Int>;            // 4 Bits, [48, 59]
  public var primaryWeaponIdx: Null<Int>;   // 4 Bits, [60, 75]
  public var secondaryWeaponIdx: Null<Int>; // 4 Bits, [60, 75]

  public function new(state: Game) {
    state.player.serialise(state.campaign, this);

    if (health == null ||
        stamina == null ||
        headIdx == null ||
        upperBodyIdx == null ||
        handsIdx == null ||
        lowerBodyIdx == null ||
        feetIdx == null ||
        primaryWeaponIdx == null ||
        secondaryWeaponIdx == null) {
      throw 'Unable to save game';
    }
  }

  // TODO: Remove once testing is done
  public function dump(): UnicodeString {
    return '$health, $stamina, $headIdx, $upperBodyIdx, $handsIdx, $lowerBodyIdx, $feetIdx, $primaryWeaponIdx, $secondaryWeaponIdx';
  }

  // Assumes all fields are non null
  public function serialize(): Bytes {
    final buffer: Bytes = Bytes.alloc(SaveDataSize);
    var offset: Int = 0;
    offset = buffer.setBitInt(offset, SaveVersion, 16);
    offset = buffer.setBitInt(offset, cast health, 7);
    offset = buffer.setBitInt(offset, cast stamina, 7);
    offset = buffer.setBitInt(offset, cast headIdx, 8);
    offset = buffer.setBitInt(offset, cast upperBodyIdx, 8);
    offset = buffer.setBitInt(offset, cast handsIdx, 8);
    offset = buffer.setBitInt(offset, cast lowerBodyIdx, 8);
    offset = buffer.setBitInt(offset, cast feetIdx, 8);
    offset = buffer.setBitInt(offset, cast primaryWeaponIdx, 8);
    offset = buffer.setBitInt(offset, cast secondaryWeaponIdx, 8);
    return buffer;
  }

  // Assumes data.length == SaveDataSize
  public function deserialise(bytes: Bytes) {
    // version = ctx.getByte() << 0x8 + ctx.getByte();
    var offset: Int = 0;
    final version: Int = bytes.getBitInt(offset, 16); offset += 16;
    if (version != SaveVersion) {
      throw 'Unexpected save version $version';
    }

    health =             bytes.getBitInt(offset, 7); offset += 7;
    stamina =            bytes.getBitInt(offset, 7); offset += 7;
    headIdx =            bytes.getBitInt(offset, 8); offset += 8;
    upperBodyIdx =       bytes.getBitInt(offset, 8); offset += 8;
    handsIdx =           bytes.getBitInt(offset, 8); offset += 8;
    lowerBodyIdx =       bytes.getBitInt(offset, 8); offset += 8;
    feetIdx =            bytes.getBitInt(offset, 8); offset += 8;
    primaryWeaponIdx =   bytes.getBitInt(offset, 8); offset += 8;
    secondaryWeaponIdx = bytes.getBitInt(offset, 8);
  }
}

function Save(state: Game): UnicodeString {
  final saveData = new SaveData(state);
  final bytes: Bytes = saveData.serialize();
  return saveData.dump() + ", " + Base85.encode(bytes);
}

// TODO: Modify game state
function Load(state: Game, str: UnicodeString): UnicodeString {
  final bytes: Bytes = Base85.decode(str, SaveDataSize);
  final saveData = new SaveData(state);
  saveData.deserialise(bytes);
  return saveData.dump();
}
