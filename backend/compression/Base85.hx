package backend.compression;

using backend.compression.ByteHelpers;
using haxe.io.Bytes;
import haxe.io.BytesBuffer;
using StringTools;

class Base85 {
  static final powers: Array<Int> = [
    Std.int(Math.pow(85, 0)),
    Std.int(Math.pow(85, 1)),
    Std.int(Math.pow(85, 2)),
    Std.int(Math.pow(85, 3)),
    Std.int(Math.pow(85, 4))
  ];

  // Compared to just [33, 118] i.e ! to u
  // `     -> v Avoid Discord code block issues
  // !!!!! -> w Reduce output size
  // !!!!  -> x Reduce output size
  // !!!   -> y Reduce output size
  // !!    -> z Reduce output size
  public static function encode(bytes: Bytes): UnicodeString {
    final buffer = new StringBuf();

    var i: Int = 0;
    while (i < bytes.length) {
      final value: UInt = bytes.getInt32Safe(i);

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

  // TODO: Remove expectedSize
  public static function decode(str: UnicodeString, expectedSize: Int): Bytes {
    final fullStr: UnicodeString = str.replace('v', '`')
                                      .replace('w', '!!!!!')
                                      .replace('x', '!!!!')
                                      .replace('y', '!!!')
                                      .replace('z', '!!');

    final buffer85: Bytes = Bytes.ofString(fullStr);
    final buffer = new BytesBuffer();

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
      return Bytes.alloc(0);
    }

    final expectedExtraByteCount: Int = longPassword.length - expectedSize;
    var extraByteCount: Int = 0;
    while (extraByteCount < expectedExtraByteCount && longPassword.fastGet(longPassword.length - extraByteCount - 1) == 0) {
      extraByteCount++;
    }

    final password: Bytes = Bytes.alloc(longPassword.length - extraByteCount);
    password.blit(0, longPassword, 0, password.length);

    if (password.length > expectedSize) {
      return Bytes.alloc(0);
    }

    return password;
  }
}
