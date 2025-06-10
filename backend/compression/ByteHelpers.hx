package backend.compression;

using haxe.io.Bytes;

class ByteHelpers {
  // Assumes pos < bytes.length
  public static function getInt32Safe(bytes: Bytes, pos: Int): Int {
    final buffer: Bytes = Bytes.alloc(4);
    buffer.fill(0, buffer.length, 0);

    final rem: Int = bytes.length - pos;
    final quot: Int = Std.int(Math.min(rem, 4));
    buffer.blit(0, bytes, pos, quot);

    return buffer.getInt32(0);
  }

  // TODO: Remove existingMiddleBits and finalMiddleBits?
  // Assumes offset and size are in bits
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

  // Assumes offset and size are in bits
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
}