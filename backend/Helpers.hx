package backend;

import haxe.ds.Either;

// From https://code.haxe.org/category/other/passing-different-types-to-a-function-parameter.html
@:nullSafety(Strict)
abstract OneOf<A, B>(Either<A, B>) from Either<A, B> to Either<A, B> {
  @:from inline static function fromA<A, B>(a: A): OneOf<A, B> {
    return Left(a);
  }
  @:from inline static function fromB<A, B>(b: B): OneOf<A, B> {
    return Right(b);
  }

  @:to inline function toA(): Null<A> {
    return switch(this) {
      case Left(a): a;
      default: null;
    }
  }
  @:to inline function toB(): Null<B> {
    return switch(this) {
      case Right(b): b;
      default: null;
    }
  }
}

class Helpers {
  public static function ThrowStr(exception: String): Void {
    #if picovision
    // TODO: Move to frontend
    Sys.print("Error: " + exception);
    while (true) {
    }
    #else
    throw new haxe.Exception(exception);
    #end
  }

  @:generic
  public static function NullCheck<T>(?value: T, exception: String): T {
    if (value == null) {
      ThrowStr(exception);
    }

    return value;
  }
}
