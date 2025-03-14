package backend.macros;

import haxe.macro.Expr;
import haxe.macro.Type;

using haxe.macro.Context;
using haxe.io.Path;
using haxe.macro.Tools;
using StringTools;
using sys.FileSystem;

class TypeGeneration {
  static var foundExtPaths: Bool = false;
  static var extPaths: Null<Map<String, Array<String>>> = [
    "Action" => [],
    "Screen" => [],
    "Screens" => []
  ];

  static function findExtPaths(directory: String = "."): Void {
    if (!directory.exists()) {
      return;
    }

    final files: Array<String> = directory.readDirectory();
    var file: String;
    for (file in files) {

      final path: String = Path.join([directory, file]);
      if (path.isDirectory()) {
        findExtPaths(path);
        continue;
      }

      var type: String;
      switch (file) {
        case "Actions.hx":
          type = "Action";
        case "Screens.hx":
          type = "Screen";
        case "ScreenInfo.hx":
          type = "Screens";
        default:
          continue;
      }

      final module: String = path.withoutExtension().replace("/", ".");
      extPaths[type].push(module);
    }

    foundExtPaths = true;
  }
  
  static public function buildGameEnum(type: String): Array<Field> {
    final enumFields: Array<Field> = [];

    if (!foundExtPaths) {
      findExtPaths();
    }

    for (enumPath in extPaths[type]) {
      final types: Array<Type> = enumPath.getModule();

      if (types.length != 1) {
        continue;
      }
      final moduleType: ModuleType = types[0].toModuleType();

      var enumType: EnumType;
      switch (moduleType) {
        case TEnumDecl(e):
          enumType = e.get();
        default:
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

  static public function buildGameMap(type: String): Array<Field> {
    final fields: Array<Field> = Context.getBuildFields();
    final mapExprs: Array<Expr> = [];

    if (!foundExtPaths) {
      findExtPaths();
    }

    for (mapPath in extPaths[type]) {
      final types: Array<Type> = mapPath.getModule();

      if (types.length != 1) {
        continue;
      }
      final moduleType: ModuleType = types[0].toModuleType();

      var classType: ClassType;
      switch (moduleType) {
        case TClassDecl(c):
          classType = c.get();
        default:
          continue;
      }

      final statics: Array<ClassField> = classType.statics.get();
      if (statics.length != 1) {
        continue;
      }

      final mapExpr: Null<TypedExpr> = statics[0].expr();
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
          case TBlock(el):
            mapItemElems = el;
          default:
            continue;
        }

        if (mapItemElems.length != 2) {
          continue;
        }
        
        var mappingRHSExpr: Expr;
        switch (mapItemElems[0].expr) {
          case TVar(_, expr):
            mappingRHSExpr = expr.getTypedExpr();
          default:
            continue;
        }

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
      access: [AStatic, APublic],
      name: type,
      kind: FVar(
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
}
