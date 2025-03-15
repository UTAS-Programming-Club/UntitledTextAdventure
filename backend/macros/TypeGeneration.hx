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
    "Screens" => [],
    "Outcome" => []
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
        case "Outcomes.hx":
          type = "Outcome";
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

      var enumType: EnumType;
      for (type in types) {
        switch (type) {
          case TEnum(e, _):
            enumType = e.get();
          default:
            continue;
        }
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

      var classType: ClassType;
      for (type in types) {
        switch (type) {
          case TInst(t, _):
            classType = t.get();
          default:
            continue;
        }
      }

      // Previously, we assumed there was only one static field per extension
      // module but module scoped functions count as statics so now we assume
      // only one static field's name ends with the type of map being created.
      final fields: Array<ClassField> = classType.statics.get();
      var mapExpr: Null<TypedExpr> = null;
      for (field in fields) {
        if (field.name.endsWith(type)) {
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

        // Everything through to mappingLHSExpr is specific to the Screens map
        // TODO: Generalise
        // new ActionScreen((cast OneOf<str, (_) -> str>), ...) to (cast OneOf<str, (_) -> str>)
        var actionBodyParam: ExprDef;
        switch (mappingRHSExpr.expr) {
          case ENew(_, params):
            actionBodyParam = params[0].expr;
          default:
            continue;
        }

        // (cast OneOf<str, (_) -> str>) to cast OneOf<str, (_) -> str>
        var actionBodyCast: ExprDef;
        switch (actionBodyParam) {
          case EMeta(_, e):
            actionBodyCast = e.expr;
          default:
            continue;
        }

        // cast OneOf<str, (_) -> str> to OneOf<str, (_) -> str>
        var actionBodyEither: ExprDef;
        switch(actionBodyCast) {
          case ECast(e, _):
            actionBodyEither = e.expr;
          default:
            continue;
        }

        // OneOf<str, (_) -> str> to str OR (_) -> str and then fix if module level function
        switch(actionBodyEither) {
          case ECall(_, params):
            params[0].expr = fixModuleStatic(params[0].expr);
          default:
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
      access: [AFinal, APublic, AStatic],
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

  static function fixModuleStatic(expr: ExprDef): ExprDef {
    switch (expr) {
      case EField(e1, field1, kind1):
        switch (e1.expr) {
          case EField(e2, field2, _):
            switch (e2.expr) {
              case EField(_, field3, _):
                final moduleStaticsClassName: String = field3 + "_Fields_";
                if (moduleStaticsClassName == field2) {
                  return EField(e2, field1, kind1);
                }
              default:
            }
          default:
        }
      default:
    }

    return expr;
  }
}
