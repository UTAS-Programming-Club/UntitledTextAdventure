package backend.macros;

import haxe.ds.Either;
import haxe.macro.Context;
import haxe.macro.Expr;
import haxe.macro.Type;

import backend.Campaign;

// From https://code.haxe.org/category/other/passing-different-types-to-a-function-parameter.html
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

macro function getCampaign(): ExprOf<Campaign> {
  final campaignName: Null<String> = Context.definedValue('campaign');
  if (campaignName == null) {
    throw ': The campaign define must be set in order to build';
  }

  final campaignModule: Null<Array<Type>> = Context.getModule('campaigns.$campaignName');
  if (campaignModule == null) {
    throw ': The requested campaign $campaignName does not exist';
  }

  var campaignStaticFields: Null<Array<ClassField>> = null;
  for (type in campaignModule) {
    switch (type) {
      case TInst(t, _):
        final campaignClass = t.get();
        if (campaignClass.name == '${campaignName}_Fields_') {
          campaignStaticFields = campaignClass.statics.get();
          break;
        }
      default:
        break;
    }
  }
  if (campaignStaticFields == null) {
    throw ': Unable to access static fields for $campaignName';
  }

  var campaignField: Null<ClassField> = null;
  for (field in campaignStaticFields) {
    switch (field.type) {
      case TLazy(f):
        switch (f()) {
          case TType(t, _) if (t.get().name == 'Campaign'):
            campaignField = field;
            break;
          default:
            break;
        }
      default:
        break;
    }
  }
  if (campaignField == null) {
    throw ': Unable to find static field for $campaignName';
  }

  final campaignTypedExpr: Null<TypedExpr> = campaignField.expr();
  if (campaignTypedExpr == null) {
    throw ': Unable to access static field for $campaignName';
  }

  return Context.storeTypedExpr(campaignTypedExpr);
}
