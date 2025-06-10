package frontends;

using backend.compression.ByteHelpers;
import backend.compression.Base85;
using haxe.io.Bytes;
import haxe.zip.Compress;
import lzstring.LZString;
using StringTools;

final SaveDataSize: Int = Math.ceil(2 * 7/8 + 7 * 5/8 + 1/8);

class SaveData {
  var health: Null<Int>;  // 7 Bits, [1, 100]
  var stamina: Null<Int>; // 7 Bits, [1, 100]

  var headKey: Null<Int>;            // 5 Bits, [00, 11]
  var upperBodyKey: Null<Int>;       // 5 Bits, [12, 23]
  var handsKey: Null<Int>;           // 5 Bits, [24, 35]
  var lowerBodyKey: Null<Int>;       // 5 Bits, [36, 47]
  var feetKey: Null<Int>;            // 5 Bits, [48, 59]
  var primaryWeaponKey: Null<Int>;   // 5 Bits, [60, 75]
  var secondaryWeaponKey: Null<Int>; // 5 Bits, [60, 75]

  var triggeredTrap: Null<Bool>; // 1 Bit

  public function new() {
    health = 100;
    stamina = 100;
    headKey = 11;
    upperBodyKey = 23;
    handsKey = 35;
    lowerBodyKey = 47;
    feetKey = 59;
    primaryWeaponKey = 60;
    secondaryWeaponKey = 75;
    triggeredTrap = true;
  }

  public function dump(): UnicodeString {
    return '$health, $stamina, $headKey, $upperBodyKey, $handsKey, $lowerBodyKey, $feetKey, $primaryWeaponKey, $secondaryWeaponKey, $triggeredTrap';
  }

  public function serialise(): Bytes {
    final buffer: Bytes = Bytes.alloc(SaveDataSize);
    var offset: Int = 0;
    offset = buffer.setBitInt(offset, health, 7);
    offset = buffer.setBitInt(offset, stamina, 7);
    offset = buffer.setBitInt(offset, headKey, 5);
    offset = buffer.setBitInt(offset, upperBodyKey - 12, 5);
    offset = buffer.setBitInt(offset, handsKey - 24, 5);
    offset = buffer.setBitInt(offset, lowerBodyKey - 34, 5);
    offset = buffer.setBitInt(offset, feetKey - 48, 5);
    offset = buffer.setBitInt(offset, primaryWeaponKey - 60, 5);
    offset = buffer.setBitInt(offset, secondaryWeaponKey - 60, 5);
    buffer.setBitInt(offset, triggeredTrap ? 1 : 0, 1);
    return buffer;
  }

  // Assumes data.length == SaveDataSize
  public function deserialise(data: Bytes): Void {
    var offset: Int = 0;
    health =             data.getBitInt(offset, 7);      offset += 7;
    stamina =            data.getBitInt(offset, 7);      offset += 7;
    headKey =            data.getBitInt(offset, 5);      offset += 5;
    upperBodyKey =       data.getBitInt(offset, 5) + 12; offset += 5;
    handsKey =           data.getBitInt(offset, 5) + 24; offset += 5;
    lowerBodyKey =       data.getBitInt(offset, 5) + 34; offset += 5;
    feetKey =            data.getBitInt(offset, 5) + 48; offset += 5;
    primaryWeaponKey =   data.getBitInt(offset, 5) + 60; offset += 5;
    secondaryWeaponKey = data.getBitInt(offset, 5) + 60; offset += 5;
    triggeredTrap =      data.getBitInt(offset, 1) == 1;
  }
}


class CompressionTest {
  static function main(): Void {
    final saveData = new SaveData();
    final saveBytes: Bytes = saveData.serialise();
    final saveHex: UnicodeString = saveBytes.toHex();
    final saveBase85: UnicodeString = Base85.encode(saveBytes);

    final l = new LZString();
    final lzCompressedBase64: UnicodeString = l.compressToBase64(saveBase85).replace('=', '');
    final lzCompressedString: UnicodeString = l.compress(saveBase85);

    final zCompressed: Bytes = Compress.run(saveBytes, 9);
    final zCompressedHex: UnicodeString = zCompressed.toHex();
    final zCompressedBase85: UnicodeString = Base85.encode(zCompressed);

    trace('    zlib as    hex, ${zCompressedHex.length}: $zCompressedHex');
    trace('    zlib as base85, ${zCompressedBase85.length}: $zCompressedBase85');
    trace('LZString as base64, ${lzCompressedBase64.length}: $lzCompressedBase64');
    trace('    data as    hex, ${saveHex.length}: $saveHex');
    trace('    data as base85, ${saveBase85.length}: $saveBase85');
    trace('LZString as string, ${lzCompressedString.length}: $lzCompressedString');


    trace('\n');
    trace('Serialise V3:         ' + saveBytes.toHex());
    trace('Round trip base85:    ' + Base85.decode(saveBase85, SaveDataSize).toHex());
    trace('Round trip lz string: ' + Base85.decode(l.decompress(lzCompressedString), SaveDataSize).toHex());


    final saveUnserialised = new SaveData();
    saveUnserialised.deserialise(saveBytes);

    trace('\n');
    trace('Save Data:                 ' + saveData.dump());
    trace('Unserialised V3 Save Data: ' + saveUnserialised.dump());
  }
}
