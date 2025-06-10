package frontends;

import haxe.crypto.Base64;
using haxe.io.Bytes;
import haxe.io.BytesBuffer;
import haxe.zip.Compress;
import lzstring.LZString;
using StringTools;

final SaveDataSizeOLD1: Int = 9 * 4 + 1;
final SaveDataSizeOLD2: Int = 10;
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

  // TODO: Remove existingMiddleBits and finalMiddleBits?
  // offset and size are in bits
  // Returns new offset in bits
  public static function setBitInt(buffer: Bytes, offset: Int, val: Int, size: Int): Int {
    final firstByte: Int = Std.int(offset / 8);
    final firstBit: Int = offset % 8;
    final finalByte: Int = firstByte + Std.int((firstBit + size - 1) / 8);
    final finalBit: Int = (firstBit + size - 1) % 8;
    if (finalByte >= buffer.length) {
      throw 'Buffer too small to fit save data.';
    }

    final firstBitCount: Int = Std.int(Math.min(8 - firstBit, size));
    final firstMask: Int = (1 << firstBitCount) - 1;
    final newFirstBits: Int = (val & firstMask) << firstBit;
    final existingFirstBits: Int = buffer.fastGet(firstByte);
    final mergedFirstBits: Int = existingFirstBits | newFirstBits;
    buffer.set(firstByte, mergedFirstBits);

    var previousBitCount: Int = firstBitCount;

    for (currentByte in (firstByte + 1)...(finalByte)) {
      final middleBitCount: Int = 8;
      final middleMask: Int = ((1 << middleBitCount) - 1) << previousBitCount;
      final newMiddleBits: Int = (val & middleMask) >> previousBitCount;
      final existingMiddleBits: Int = buffer.fastGet(currentByte);
      final mergedMiddleBits: Int = existingMiddleBits | newMiddleBits;
      buffer.set(currentByte, mergedMiddleBits);

      previousBitCount += middleBitCount;
    }

    final finalBitCount: Int = size - previousBitCount;
    if (finalBitCount > 8) {
      throw 'Unable to store final byte in save data buffer.';
    }

    final finalMask: Int = ((1 << finalBitCount) - 1) << previousBitCount;
    final newFinalBits: Int = (val & finalMask) >> previousBitCount;
    final existingFinalBits: Int = buffer.fastGet(finalByte);
    final mergedFinalBits: Int = existingFinalBits | newFinalBits;
    buffer.set(finalByte, mergedFinalBits);

    return finalByte * 8 + finalBit + 1;
  }

  public static function getBitInt(buffer: Bytes, offset: Int, size: Int): Int {
    final firstByte: Int = Std.int(offset / 8);
    final firstBit: Int = offset % 8;
    final finalByte: Int = firstByte + Std.int((firstBit + size - 1) / 8);
    if (finalByte >= buffer.length) {
      throw 'Buffer too small to read save data.';
    }

    var val: Int = 0;

    final firstBits: Int = buffer.fastGet(firstByte);
    final firstBitCount: Int = Std.int(Math.min(8 - firstBit, size));
    final firstMask: Int = (1 << firstBitCount) - 1;
    val |= (firstBits >> firstBit) & firstMask;

    var previousBitCount: Int = firstBitCount;

    for (currentByte in (firstByte + 1)...(finalByte)) {
      final middleBits: Int = buffer.fastGet(currentByte);
      val |= middleBits << previousBitCount;

      previousBitCount += 8;
    }

    final finalBitCount: Int = size - previousBitCount;
    if (finalBitCount > 8) {
      throw 'Unable to read final byte from save data buffer.';
    }

    final finalBits: Int = buffer.fastGet(finalByte);
    final finalMask: Int = ((1 << finalBitCount) - 1) << previousBitCount;
    val |= (finalBits << previousBitCount) & finalMask;

    return val;
  }

  public function serialiseOLD1(): Bytes {
    final buffer = new BytesBuffer();
    buffer.addInt32(health);
    buffer.addInt32(stamina);
    buffer.addInt32(headKey);
    buffer.addInt32(upperBodyKey);
    buffer.addInt32(handsKey);
    buffer.addInt32(lowerBodyKey);
    buffer.addInt32(feetKey);
    buffer.addInt32(primaryWeaponKey);
    buffer.addInt32(secondaryWeaponKey);
    buffer.addByte(triggeredTrap ? 1 : 0);
    return buffer.getBytes();
  }

  public function serialiseOLD2(): Bytes {
    final buffer = new BytesBuffer();
    buffer.addByte(health);
    buffer.addByte(stamina);
    buffer.addByte(headKey);
    buffer.addByte(upperBodyKey);
    buffer.addByte(handsKey);
    buffer.addByte(lowerBodyKey);
    buffer.addByte(feetKey);
    buffer.addByte(primaryWeaponKey);
    buffer.addByte(secondaryWeaponKey);
    buffer.addByte(triggeredTrap ? 1 : 0);
    return buffer.getBytes();
  }

  public function serialise(): Bytes {
    final buffer: Bytes = Bytes.alloc(SaveDataSize);
    var offset: Int = 0;
    offset = setBitInt(buffer, offset, health, 7);
    offset = setBitInt(buffer, offset, health, 7);
    offset = setBitInt(buffer, offset, headKey, 5);
    offset = setBitInt(buffer, offset, upperBodyKey - 12, 5);
    offset = setBitInt(buffer, offset, handsKey - 24, 5);
    offset = setBitInt(buffer, offset, lowerBodyKey - 34, 5);
    offset = setBitInt(buffer, offset, feetKey - 48, 5);
    offset = setBitInt(buffer, offset, primaryWeaponKey - 60, 5);
    offset = setBitInt(buffer, offset, secondaryWeaponKey - 60, 5);
    setBitInt(buffer, offset, triggeredTrap ? 1 : 0, 1);
    return buffer;
  }

  // data.length == SaveDataSizeOLD1
  public function deserialiseOLD1(data: Bytes): Void {
    health =             data.getInt32(0);
    stamina =            data.getInt32(4);
    headKey =            data.getInt32(8);
    upperBodyKey =       data.getInt32(12);
    handsKey =           data.getInt32(16);
    lowerBodyKey =       data.getInt32(20);
    feetKey =            data.getInt32(24);
    primaryWeaponKey =   data.getInt32(28);
    secondaryWeaponKey = data.getInt32(32);
    triggeredTrap =      data.fastGet(36) == 1;
  }

  // data.length == SaveDataSizeOLD2
  public function deserialiseOLD2(data: Bytes): Void {
    health =             data.fastGet(0);
    stamina =            data.fastGet(1);
    headKey =            data.fastGet(2);
    upperBodyKey =       data.fastGet(3);
    handsKey =           data.fastGet(4);
    lowerBodyKey =       data.fastGet(5);
    feetKey =            data.fastGet(6);
    primaryWeaponKey =   data.fastGet(7);
    secondaryWeaponKey = data.fastGet(8);
    triggeredTrap =      data.fastGet(9) == 1;
  }

  // data.length == SaveDataSize
  public function deserialise(data: Bytes): Void {
    var offset: Int = 0;
    health =             getBitInt(data, offset, 7);      offset += 7;
    health =             getBitInt(data, offset, 7);      offset += 7;
    headKey =            getBitInt(data, offset, 5);      offset += 5;
    upperBodyKey =       getBitInt(data, offset, 5) + 12; offset += 5;
    handsKey =           getBitInt(data, offset, 5) + 24; offset += 5;
    lowerBodyKey =       getBitInt(data, offset, 5) + 34; offset += 5;
    feetKey =            getBitInt(data, offset, 5) + 48; offset += 5;
    primaryWeaponKey =   getBitInt(data, offset, 5) + 60; offset += 5;
    secondaryWeaponKey = getBitInt(data, offset, 5) + 60; offset += 5;
    triggeredTrap =      getBitInt(data, offset, 1) == 1;
  }
}

// pos < bytes.length
function getInt32Safe(bytes: Bytes, pos: Int): Int {
  final buffer: Bytes = Bytes.alloc(4);
  buffer.fill(0, buffer.length, 0);

  final rem: Int = bytes.length - pos;
  final quot: Int = Std.int(Math.min(rem, 4));
  buffer.blit(0, bytes, pos, quot);

  return buffer.getInt32(0);
}

// Compared to just [33, 118] i.e ! to u
// `     -> v Avoid Discord code block issues
// !!!!! -> w Reduce output size
// !!!!  -> x Reduce output size
// !!!   -> y Reduce output size
// !!    -> z Reduce output size
function base85encode(bytes: Bytes): UnicodeString {
  final buffer = new StringBuf();
  final powers: Array<Int> = [
    Std.int(Math.pow(85, 0)),
    Std.int(Math.pow(85, 1)),
    Std.int(Math.pow(85, 2)),
    Std.int(Math.pow(85, 3)),
    Std.int(Math.pow(85, 4))
  ];

  var i: Int = 0;
  while (i < bytes.length) {
    final value: UInt = getInt32Safe(bytes, i);

    final digit4: Int = Std.int(value / powers[4]);
    final value4: Int = value - digit4 * powers[4];

    final digit3: Int = Std.int(value4 / powers[3]);
    final value3: Int = value4 - digit3 * powers[3];

    final digit2: Int = Std.int(value3 / powers[2]);
    final value2: Int = value3 - digit2 * powers[2];

    final digit1: Int = Std.int(value2 / powers[1]);
    final value1: Int = value2 - digit1 * powers[1];

    final digit0: Int = Std.int(value1 / powers[0]);

    // 33 is '!', first used char
    buffer.addChar(digit0 + 33);
    buffer.addChar(digit1 + 33);
    buffer.addChar(digit2 + 33);
    buffer.addChar(digit3 + 33);
    buffer.addChar(digit4 + 33);

    i += 4;
  }

  var base85 = buffer.toString();
  var extraCharCount: Int = 0;
  while (extraCharCount < 4 && base85.charAt(base85.length - extraCharCount - 1) == '!') {
    extraCharCount++;
  }

  return base85.substr(0, base85.length - extraCharCount)
               .replace('`', 'v')
               .replace('!!!!!', 'w')
               .replace('!!!!', 'x')
               .replace('!!!', 'y')
               .replace('!!', 'z');
}

function base85decode(str: UnicodeString, expectedSize: Int): Bytes {
  final fullStr: UnicodeString = str.replace('v', '`').replace('w', '!!!!!').replace('x', '!!!!').replace('y', '!!!').replace('z', '!!');
  final buffer85: Bytes = Bytes.ofString(fullStr);
  final buffer = new BytesBuffer();
  final powers: Array<Int> = [
    Std.int(Math.pow(85, 0)),
    Std.int(Math.pow(85, 1)),
    Std.int(Math.pow(85, 2)),
    Std.int(Math.pow(85, 3)),
    Std.int(Math.pow(85, 4))
  ];

  var i: Int = 0;
  while (i < buffer85.length) {
    final tempBuffer: Bytes = Bytes.alloc(5);
    tempBuffer.fill(0, 5, '!'.code);

    final rem: Int = buffer85.length - i;
    final quot: Int = Std.int(Math.min(rem, 5));
    tempBuffer.blit(0, buffer85, i, quot);

    final digit0: Int = tempBuffer.fastGet(0) - 33;
    final digit1: Int = tempBuffer.fastGet(1) - 33;
    final digit2: Int = tempBuffer.fastGet(2) - 33;
    final digit3: Int = tempBuffer.fastGet(3) - 33;
    final digit4: Int = tempBuffer.fastGet(4) - 33;

    buffer.addInt32(digit4 * powers[4] + digit3 * powers[3] + digit2 * powers[2] + digit1 * powers[1] + digit0 * powers[0]);

    i += 5;
  }

  final longPassword: Bytes = buffer.getBytes();
  if (longPassword.length < expectedSize) {
    throw 'Password too short';
  }

  final expectedExtraByteCount: Int = longPassword.length - expectedSize;
  var extraByteCount: Int = 0;
  while (extraByteCount < expectedExtraByteCount && longPassword.fastGet(longPassword.length - extraByteCount - 1) == 0) {
    extraByteCount++;
  }

  final password: Bytes = Bytes.alloc(longPassword.length - extraByteCount);
  password.blit(0, longPassword, 0, password.length);

  if (password.length > expectedSize) {
    throw 'Password too long';
  }

  return password;
}


class CompressionTest {
  static function main(): Void {
    final saveData = new SaveData();
    final saveBytes: Bytes = saveData.serialise();
    final saveHex: UnicodeString = saveBytes.toHex();
    final saveBase64: UnicodeString = Base64.encode(saveBytes, false);
    final saveBase85: UnicodeString = base85encode(saveBytes);

    final l = new LZString();
    final lzCompressedBase64: UnicodeString = l.compressToBase64(saveBase85).replace('=', '');
    final lzCompressedString: UnicodeString = l.compress(saveBase85);

    final zCompressed: Bytes = Compress.run(saveBytes, 9);
    final zCompressedHex: UnicodeString = zCompressed.toHex();
    final zCompressedBase64: UnicodeString = Base64.encode(zCompressed, false);
    final zCompressedBase85: UnicodeString = base85encode(zCompressed);

    trace('    zlib as    hex, ${zCompressedHex.length}: $zCompressedHex');
    trace('    zlib as base64, ${zCompressedBase64.length}: $zCompressedBase64');
    trace('    zlib as base85, ${zCompressedBase85.length}: $zCompressedBase85');
    trace('LZString as base64, ${lzCompressedBase64.length}: $lzCompressedBase64');
    trace('    data as    hex, ${saveHex.length}: $saveHex');
    trace('    data as base64, ${saveBase64.length}: $saveBase64');
    trace('    data as base85, ${saveBase85.length}: $saveBase85');
    trace('LZString as string, ${lzCompressedString.length}: $lzCompressedString');


    final saveBytesOLD1: Bytes = saveData.serialiseOLD1();
    final saveBytesOLD2: Bytes = saveData.serialiseOLD2();

    trace('\n');
    trace('Serialise V1:         ' + saveBytesOLD1.toHex());
    trace('Serialise V2:         ' + saveBytesOLD2.toHex());
    trace('Current version:      ' + saveBytes.toHex());
    trace('Round trip base85:    ' + base85decode(saveBase85, SaveDataSize).toHex());
    trace('Round trip lz base85: ' + base85decode(l.decompress(lzCompressedString), SaveDataSize).toHex());


    final saveBase85OLD1: UnicodeString = base85encode(saveBytesOLD1);
    final saveUnencodedOLD1: Bytes = base85decode(saveBase85OLD1, SaveDataSizeOLD1);
    final saveUnserialisedOLD1 = new SaveData();
    saveUnserialisedOLD1.deserialiseOLD1(saveUnencodedOLD1);

    final saveBase85OLD2: UnicodeString = base85encode(saveBytesOLD2);
    final saveUnencodedOLD2: Bytes = base85decode(saveBase85OLD2, SaveDataSizeOLD2);
    final saveUnserialisedOLD2 = new SaveData();
    saveUnserialisedOLD2.deserialiseOLD2(saveUnencodedOLD2);

    final saveUnserialised = new SaveData();
    saveUnserialised.deserialise(saveBytes);

    trace('\n');
    trace('Save Data:               ' + saveData.dump());
    trace('Unserialised V1 Save Data: ' + saveUnserialisedOLD1.dump());
    trace('Unserialised V2 Save Data: ' + saveUnserialisedOLD2.dump());
    trace('Unserialised V3 Save Data: ' + saveUnserialised.dump());
  }
}
