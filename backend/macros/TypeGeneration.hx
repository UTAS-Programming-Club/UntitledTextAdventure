package backend.macros;

import haxe.macro.Expr;
using haxe.macro.ExprTools;
using StringTools;

class TypeGeneration {
  public static function fixModuleStatics(expr: Expr): Expr {
    return switch (expr.expr) {
      case EField(e, field, _) if (field.endsWith("_Fields_")):
        e;
      default:
        expr.map(fixModuleStatics);
    }
  }
}
