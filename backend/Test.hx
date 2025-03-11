package backend;

import haxe.macro.Context;
import haxe.macro.Expr;
import haxe.macro.Type;
using haxe.macro.Tools;
// using haxe.io.Path;
// using sys.FileSystem;

class TypeBuildingMacro {
  static final extensionInfos: Array<{path: String, fieldName: String}> = [{
    path: "backend.CoreGame",
    fieldName: "CoreGame"
  }];
  static final extensionsDir: String = "extensions";
  static final generatedModule: String = "game.generated.Generated";

  macro static public function generateEnum(name: String, idx: Int): Array<Field> {
    final arrayItemExprs: Array<Expr> = [];

    for (extensionInfo in extensionInfos) {
      final objectExpr: Null<TypedExprDef> = getExtensionObject(extensionInfo, idx);
      trace(name, idx, extensionInfo);
    }


    final noArgs = makeEnumField("A", FVar(null, null));
    final eFunc = macro function(value:Int) {};
    final fInt = switch (eFunc.expr) {
      case EFunction(_, f): f;
      case _: throw "false";
    }
    final intArg = makeEnumField("B", FFun(fInt));
    return [noArgs, intArg];
  }

  static function getExtensionObject(
    extensionInfo: {path: String, fieldName: String}, idx: Int
  ): Null<TypedExprDef> {
    var moduleTypes: Array<Type>;

    try {
      moduleTypes = Context.getModule(extensionInfo.path);
    } catch (e: haxe.macro.Error) {
      if (e.message == "Type not found : " + extensionInfo.path) {
        return null;
      }

      throw e;
    }

    var fieldExpr: Null<TypedExprDef> = null;
    for (type in moduleTypes) {
      final moduleType: ModuleType = type.toModuleType();

      var moduleClass: ClassType;
      switch (moduleType) {
        case TClassDecl(c):
          moduleClass = c.get();
        default:
          continue;
      }

      switch (moduleClass.kind) {
        case KModuleFields(_):
          final field: Null<ClassField> = moduleClass.findField(
            extensionInfo.fieldName, true
          );
          fieldExpr = field?.expr().expr;
          break;
        default:
          continue;
      }

      trace(fieldExpr);
    }

    return null;
  }


  static function makeEnumField(name, kind): Field {
    return {
      name: name,
      doc: null,
      meta: [],
      access: [],
      kind: kind,
      pos: Context.currentPos()
    }
  }
}
