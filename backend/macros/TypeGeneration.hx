package backend.macros;

import haxe.macro.Expr;
import haxe.macro.Type;

using haxe.io.Path;
using haxe.macro.Context;
using haxe.macro.ExprTools;
using StringTools;
using sys.FileSystem;

class TypeGeneration {
  static var foundFilePaths: Bool = false;
  static var filePaths: Map<String, Array<String>> = [];

  static function findFilePaths(directory: String = "."): Void {
    if (!directory.exists()) {
      return;
    }

    final files: Array<String> = directory.readDirectory();
    var file: String;
    for (file in files) {
      final path: String = Path.join([directory, file]);

      if (path.isDirectory()) {
        findFilePaths(path);
        continue;
      }

      if (!file.endsWith(".hx")) {
        continue;
      }

      if (filePaths[file] == null) {
        filePaths[file] = [];
      }

      final module: String = path.withoutExtension().replace("/", ".");
      filePaths[file].push(module);
    }

    foundFilePaths = true;
  }
  
  static public function buildGameEnum(fileName: String): Array<Field> {
    final enumFields: Array<Field> = [];

    if (!foundFilePaths) {
      findFilePaths();
    }

    if (filePaths[fileName] == null) {
      return enumFields;
    }

    for (enumPath in filePaths[fileName]) {
      final types: Array<Type> = enumPath.getModule();

      // TODO: Check type name
      var enumType: Null<EnumType> = null;
      for (type in types) {
        switch (type) {
          case TEnum(e, _):
            enumType = e.get();
          default:
            continue;
        }
      }

      if (enumType == null) {
        continue;
      }

      for (construct in enumType.constructs) {
        switch (construct.type) {
          case TEnum(_, _):
            enumFields.push(makeEnumField(construct.name, FVar(null)));
          case TFun(args, _):
            final func: Function = {
              args: args.map(
                function(arg): FunctionArg return {
                  name: arg.name,
                  type: arg.t.toComplexType()
                }
              )
            };

            enumFields.push(makeEnumField(construct.name, FFun(func)));
          default:
            trace("Unexpected enum constructor type: " + construct.type);
        }
      }
    }

    return enumFields;
  }

  static public function buildGameMap(fileName: String, typeName: String): Array<Field> {
    final fields: Array<Field> = Context.getBuildFields();
    final mapExprs: Array<Expr> = [];

    if (!foundFilePaths) {
      findFilePaths();
    }

    if (filePaths[fileName] == null) {
      return fields;
    }

    for (mapPath in filePaths[fileName]) {
      final types: Array<Type> = mapPath.getModule();

      // TODO: Check type name
      var classType: Null<ClassType> = null;
      for (type in types) {
        switch (type) {
          case TInst(t, _):
            classType = t.get();
          default:
            continue;
        }
      }

      if (classType == null) {
        continue;
      }

      // Previously, we assumed there was only one static field per extension
      // module but module scoped functions count as statics so now we assume
      // only one static field's name ends with the type of map being created.
      final fields: Array<ClassField> = classType.statics.get();
      var mapExpr: Null<TypedExpr> = null;
      for (field in fields) {
        if (field.name.endsWith(typeName)) {
          mapExpr = field.expr();
          break;
        }
      }

      if (mapExpr == null) {
        continue;
      }

      var mapItems: Array<TypedExpr>;
      switch (mapExpr.expr) {
        case TBlock(el):
          mapItems = el;
        default:
          continue;
      }

      for (i in 1...(mapItems.length - 1)) {
        var mapItemElems: Array<TypedExpr>;
        switch (mapItems[i].expr) {
          case TBlock(el) if (el.length == 2):
            mapItemElems = el;
          default:
            continue;
        }

        var mappingRHSExpr: Expr;
        switch (mapItemElems[0].expr) {
          case TVar(_, expr):
            mappingRHSExpr = expr.getTypedExpr();
          default:
            continue;
        }

        mappingRHSExpr = fixModuleStatics(mappingRHSExpr);

        var mappingLHSExpr: Expr;
        switch (mapItemElems[1].expr) {
          case TCall(_, el):
            mappingLHSExpr = el[0].getTypedExpr();
          default:
            continue;
        }

        mapExprs.push(
          macro $mappingLHSExpr => $mappingRHSExpr
        );
      }
    }

    fields.push({
      access: [AFinal, APublic, AStatic],
      name: typeName,
      kind: FVar(
        // TODO: Remove these types to allow generating other maps e.g. equipment
        macro: Map<GameScreen, Screen>, macro $a{mapExprs}
      ),
      pos: Context.currentPos()
    });

    return fields;
  }

  static function makeEnumField(name: String, kind: FieldType): Field return {
    kind: kind,
    name: name,
    pos: Context.currentPos()
  }

  static function fixModuleStatics(expr: Expr): Expr {
    return switch (expr.expr) {
      case EField(e, field, _) if (field.endsWith("_Fields_")):
        e;
      default:
        expr.map(fixModuleStatics);
    }
  }
}
