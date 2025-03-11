package backend;

import haxe.macro.Context;
import haxe.macro.Expr;
import haxe.macro.Type;
using haxe.io.Path;
using haxe.macro.Tools;
using StringTools;
using sys.FileSystem;

class Macros2 {
  static final enumPaths: Map<String, Array<String>> = [
    "Screens" => [
      "backend.coregame.Screens",
      "extensions.testextension.Screens"
    ]
  ];

  /*static function findEnums(directory: String): Void {
    if (!directory.exists()) {
      return;
    }

    final files: Array<String> = directory.readDirectory();
    var file: String;
    for (file in files) {

      final path: String = Path.join([directory, file]);
      if (path.isDirectory()) {
        findEnums(path);
        continue;
      }

      if (!path.endsWith(".hx")) {
        continue;
      }

      final module: String = path.withoutExtension().replace("/", ".");
      Context.onAfterInitMacros(function() processEnums(module));
    }
  }

  static function processEnums(module: String) {
    return;
    final types: Array<Type> = Context.getModule(module);

    for (type in types) {
      var test;
      switch (type) {
        case TEnum(t, _):
          final metadata: MetaAccess = t.get().meta;
          if (metadata.has(":GameActionsEnum")) {
            // enumPaths["Actions"].push(t.toString());
            // trace(enumPaths);
          }
        default:
          continue;
      }

      // trace(module + ": " + moduleType);
    }
  }*/

  // static function createGameEnums1() {
  //   haxe.macro.Context.onAfterInitMacros(createGameEnums3);
  // }

  static function createGameEnums2() {
    haxe.macro.Context.onAfterInitMacros(createGameEnums4);
  }

  // static function createGameEnums3() {
  //   Context.defineModule("backend.generated1.Actions", [
  //     {
  //       fields: buildGameEnum2("Actions"),
  //       kind: TDEnum,
  //       name: "GameActions",
  //       pack: ["backend", "generated1"],
  //       pos: Context.currentPos()
  //     }
  //   ]);
  // }

  static function createGameEnums4() {
    Context.defineModule("backend.generated2.Screens", [
      {
        fields: buildGameEnum2("Screens"),
        kind: TDEnum,
        name: "GameScreens",
        pack: ["backend", "generated2"],
        pos: Context.currentPos()
      }
    ]);
  }

  /*macro static public function addGameEnum(type: String, expr: Expr): Null<Array<Field>> {
    // enumPaths[type].push(Context.getPosInfos(expr.pos).file.split(".")[0].replace("/", "."));
    // trace("addGameEnum: " + Context.getPosInfos(expr.pos).file.split(".")[0].replace("/", "."));
    return null;
  }*/

  /*macro static public function buildGameEnum(type: String): Array<Field> {
    final enumFields: Array<Field> = [];

    for (enumPath in enumPaths[type]) {
      // trace("buildGameEnum: " + type, enumPaths[type]);
      final types: Array<Type> = Context.getModule(enumPath);

      for (type in types) {
        final moduleType: ModuleType = type.toModuleType();

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
    }

    return enumFields;
  }*/
  
  static public function buildGameEnum2(type: String): Array<Field> {
    final enumFields: Array<Field> = [];

    for (enumPath in enumPaths[type]) {
      // trace("buildGameEnum: " + type, enumPaths[type]);
      final types: Array<Type> = Context.getModule(enumPath);

      for (type in types) {
        final moduleType: ModuleType = type.toModuleType();

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
    }

    return enumFields;
  }

  static function makeEnumField(name: String, kind: FieldType): Field return {
    kind: kind,
    name: name,
    pos: Context.currentPos()
  }
}
