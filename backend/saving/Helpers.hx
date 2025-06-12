package backend.saving;

import haxe.io.Bytes;

class Helpers {
  // Assumes pos < bytes.length
  public static function getInt32Safe(bytes: Bytes, pos: Int): Int {
    final buffer: Bytes = Bytes.alloc(4);
    buffer.fill(0, buffer.length, 0);

    final rem: Int = bytes.length - pos;
    final quot: Int = Std.int(Math.min(rem, 4));
    buffer.blit(0, bytes, pos, quot);

    return buffer.getInt32(0);
  }


  // Assumes offset and size are in bits and buffer[i] for i > offset % 8 is safe to overrride
  // Returns new offset in bits
  public static function setBitInt(buffer: Bytes, offset: Int, val: Int, size: Int): Int {
    final firstByte: Int = Std.int(offset / 8);
    final firstBit: Int = offset % 8;
    final finalByte: Int = firstByte + Std.int((firstBit + size - 1) / 8);
    final finalBit: Int = (firstBit + size - 1) % 8;
    if (finalByte >= buffer.length) {
      throw 'Buffer too small to fit data.';
    }

    final firstBitCount: Int = Std.int(Math.min(8 - firstBit, size));
    final firstMask: Int = (1 << firstBitCount) - 1;
    final newFirstBits: Int = (val & firstMask) << firstBit;
    final existingFirstBits: Int = buffer.get(firstByte);
    final mergedFirstBits: Int = existingFirstBits | newFirstBits;
    buffer.set(firstByte, mergedFirstBits);

    var previousBitCount: Int = firstBitCount;

    for (currentByte in (firstByte + 1)...(finalByte)) {
      final middleBitCount: Int = 8;
      final middleMask: Int = ((1 << middleBitCount) - 1) << previousBitCount;
      final newMiddleBits: Int = (val & middleMask) >> previousBitCount;
      buffer.set(currentByte, newMiddleBits);

      previousBitCount += middleBitCount;
    }

    final finalBitCount: Int = size - previousBitCount;
    if (finalBitCount > 8) {
      throw 'Unable to store final byte in data buffer.';
    }

    if (finalByte != firstByte) {
      final finalMask: Int = ((1 << finalBitCount) - 1) << previousBitCount;
      final newFinalBits: Int = (val & finalMask) >> previousBitCount;
      buffer.set(finalByte, newFinalBits);
    }

    return finalByte * 8 + finalBit + 1;
  }

  // Assumes offset and size are in bits
  public static function getBitInt(buffer: Bytes, offset: Int, size: Int): Int {
    final firstByte: Int = Std.int(offset / 8);
    final firstBit: Int = offset % 8;
    final finalByte: Int = firstByte + Std.int((firstBit + size - 1) / 8);
    if (finalByte >= buffer.length) {
      throw 'Buffer too small to read data from.';
    }

    var val: Int = 0;

    final firstBits: Int = buffer.get(firstByte);
    final firstBitCount: Int = Std.int(Math.min(8 - firstBit, size));
    final firstMask: Int = (1 << firstBitCount) - 1;
    val |= (firstBits >> firstBit) & firstMask;

    var previousBitCount: Int = firstBitCount;

    for (currentByte in (firstByte + 1)...(finalByte)) {
      final middleBits: Int = buffer.get(currentByte);
      val |= middleBits << previousBitCount;

      previousBitCount += 8;
    }

    final finalBitCount: Int = size - previousBitCount;
    if (finalBitCount > 8) {
      throw 'Unable to read final byte from data buffer.';
    }

    final finalBits: Int = buffer.get(finalByte);
    final finalMask: Int = ((1 << finalBitCount) - 1) << previousBitCount;
    val |= (finalBits << previousBitCount) & finalMask;

    return val;
  }


  public static function EnumToInt(e: EnumValue): Int {
    if (Type.enumParameters(e).length > 0) {
      throw 'Cannot convert enums with parameters to ints.';
    }
    return Type.enumIndex(e);
  }

  public static function IntToEnum<T>(e: Enum<T>, val: Int): T {
    return Type.createEnumIndex(e, val, null);
  }
}
