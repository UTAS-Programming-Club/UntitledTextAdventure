package frontends;

import haxe.crypto.Base64;
import haxe.io.Bytes;
import haxe.io.BytesBuffer;
import haxe.zip.Compress;
import lzstring.LZString;
using StringTools;

final SaveDataSizeOLD1: Int = 9 * 8 + 1;
final SaveDataSizeOLD2: Int = 10;
final SaveDataSize: Int = Math.ceil(2 * 7/8 + 7 * 5/8 + 1/8);

class SaveData {
  final health: Null<Int>;  // 7 Bits, [1, 100]
  final stamina: Null<Int>; // 7 Bits, [1, 100]

  final headKey: Null<Int>;            // 5 Bits, [00, 11]
  final upperBodyKey: Null<Int>;       // 5 Bits, [12, 23]
  final handsKey: Null<Int>;           // 5 Bits, [24, 35]
  final lowerBodyKey: Null<Int>;       // 5 Bits, [36, 47]
  final feetKey: Null<Int>;            // 5 Bits, [48, 59]
  final primaryWeaponKey: Null<Int>;   // 5 Bits, [60, 75]
  final secondaryWeaponKey: Null<Int>; // 5 Bits, [60, 75]

  final triggeredTrap: Null<Bool>; // 1 Bit

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

  // TODO: Remove existingMiddleBits and finalMiddleBits?
  // offset and size are in bits
  // Returns new offset in bits
  public static function addBitInt(buffer: Bytes, offset: Int, val: Int, size: Int): Int {
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
    final existingFirstBits: Int = Bytes.fastGet(buffer, firstByte);
    final mergedFirstBits: Int = existingFirstBits | newFirstBits;
    buffer.set(firstByte, mergedFirstBits);

    var previousBitCount: Int = firstBitCount;

    for (currentByte in (firstByte + 1)...(finalByte)) {
      final middleBitCount: Int = 8;
      final middleMask: Int = ((1 << middleBitCount) - 1) << previousBitCount;
      final newMiddleBits: Int = (val & middleMask) >> previousBitCount;
      final existingMiddleBits: Int = Bytes.fastGet(buffer, currentByte);
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
    final existingFinalBits: Int = Bytes.fastGet(buffer, finalByte);
    final mergedFinalBits: Int = existingFinalBits | newFinalBits;
    buffer.set(finalByte, mergedFinalBits);

    return finalByte * 8 + finalBit + 1;
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
    offset = addBitInt(buffer, offset, health, 7);
    offset = addBitInt(buffer, offset, health, 7);
    offset = addBitInt(buffer, offset, headKey, 5);
    offset = addBitInt(buffer, offset, upperBodyKey - 12, 5);
    offset = addBitInt(buffer, offset, handsKey - 24, 5);
    offset = addBitInt(buffer, offset, lowerBodyKey - 34, 5);
    offset = addBitInt(buffer, offset, feetKey - 48, 5);
    offset = addBitInt(buffer, offset, primaryWeaponKey - 60, 5);
    offset = addBitInt(buffer, offset, secondaryWeaponKey - 60, 5);
    addBitInt(buffer, offset, triggeredTrap ? 1 : 0, 1);
    return buffer;
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

  if (extraCharCount > 0) {
    final shorterBuffer = new StringBuf();
    shorterBuffer.addSub(base85, 0, base85.length - extraCharCount);
    base85 = shorterBuffer.toString();
  }

  return base85.replace('`', 'v').replace('!!!!!', 'w').replace('!!!!', 'x').replace('!!!', 'y').replace('!!', 'z');
}

function base85decode(str: UnicodeString, expectedSize: Int): Bytes {
  final base85: UnicodeString = str.replace('v', '`').replace('w', '!!!!!').replace('x', '!!!!').replace('y', '!!!').replace('z', '!!');
  final buffer85: Bytes = Bytes.ofString(base85);
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

    final digit0: Int = Bytes.fastGet(tempBuffer, 0) - 33;
    final digit1: Int = Bytes.fastGet(tempBuffer, 1) - 33;
    final digit2: Int = Bytes.fastGet(tempBuffer, 2) - 33;
    final digit3: Int = Bytes.fastGet(tempBuffer, 3) - 33;
    final digit4: Int = Bytes.fastGet(tempBuffer, 4) - 33;

    buffer.addInt32(digit4 * powers[4] + digit3 * powers[3] + digit2 * powers[2] + digit1 * powers[1] + digit0 * powers[0]);

    i += 5;
  }
  final longPassword: Bytes = buffer.getBytes();

  if (longPassword.length < expectedSize) {
    throw 'Password too short';
  }

  final expectedExtraByteCount: Int = longPassword.length - expectedSize;
  var extraByteCount: Int = 0;
  while (extraByteCount < expectedExtraByteCount && Bytes.fastGet(longPassword, longPassword.length - extraByteCount - 1) == 0) {
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
    final saveBytesBase85: UnicodeString = base85encode(saveBytes);


    final l = new LZString();
    final lzCompressedBase64: UnicodeString = l.compressToBase64(saveBytes.toHex()).replace('=', '');
    final lzCompressedString: UnicodeString = l.compress(saveBytes.toHex());

    final zCompressed: Bytes = Compress.run(saveBytes, 9);
    final zCompressedHex: UnicodeString = zCompressed.toHex();
    final zCompressedBase64: UnicodeString = Base64.encode(zCompressed, false);
    final zCompressedBase85: UnicodeString = base85encode(zCompressed);

    trace('    zlib as    hex, ${zCompressedHex.length}: $zCompressedHex');
    trace('LZString as base64, ${lzCompressedBase64.length}: $lzCompressedBase64');
    trace('    zlib as base64, ${zCompressedBase64.length}: $zCompressedBase64');
    trace('    zlib as base85, ${zCompressedBase85.length}: $zCompressedBase85');
    trace('    data as    hex, ${saveHex.length}: $saveHex');
    trace('    data as base64, ${saveBase64.length}: $saveBase64');
    trace('LZString as string, ${lzCompressedString.length}: $lzCompressedString');
    trace('    data as base85, ${saveBytesBase85.length}: $saveBytesBase85');

    trace('\n');
    trace(saveData.serialiseOLD1().toHex());
    trace(saveData.serialiseOLD2().toHex());
    trace(saveBytes.toHex());
    trace(base85decode(base85encode(saveBytes), SaveDataSize).toHex());
  }
}
