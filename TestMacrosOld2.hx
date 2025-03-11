import haxe.macro.Context;
import haxe.macro.Expr;
using haxe.macro.TypeTools;
using haxe.io.Path;
using sys.FileSystem;

class GameGeneration {
  static final extensionModuleInfos: Array<{module: String, roomsEnumName: String}> = [];
  static final extensionIncludes: Array<ImportExpr> = [];

  static final coreRoomsModuleInfo = {
    module: "game.Rooms",
    roomsEnumName: "Rooms"
  };
  static final extensionRoomsDir: String = "extensions";

  static public function generateRoomsEnum(): Void {
    extensionModuleInfos.push(coreRoomsModuleInfo);
    extensionIncludes.push({
      path: [{
        pos: Context.currentPos(),
        name: "game"
      }, {
        pos: Context.currentPos(),
        name: "Rooms"
      }], 
      mode: INormal
    });

    final extensionFiles: Array<String> = extensionRoomsDir.readDirectory();
    for (extensionFile in extensionFiles) {
      if (Path.extension(extensionFile) != "hx") {
        continue;
      }

      final extensionPath = Path.join([extensionRoomsDir, extensionFile]);
      var extensionModule = extensionPath.withoutExtension();
      extensionModule = ~/[\\\/]/g.replace(extensionModule, ".");
      extensionModuleInfos.push({
        module: extensionModule,
        roomsEnumName: extensionPath.withoutDirectory().withoutExtension() + "Rooms"
      });

      // extensionIncludes.push({
      //   path: [{
      //     pos: Context.currentPos(),
      //     name: "extensions"
      //   }, {
      //     pos: Context.currentPos(),
      //     name: extensionPath.withoutDirectory().withoutExtension(),
      //   }], 
      //   mode: INormal
      // });
    }

    Context.onAfterInitMacros(generateRoomsEnumInternal);
  }

  static function generateRoomsEnumInternal(): Void {
    Context.defineModule("extensions.Generated", [], extensionIncludes, null);

    final roomFields: Array<Field> = [];

    for (extensionModuleInfo in extensionModuleInfos) {
      final enumType: haxe.macro.Type = Context.getType(
        extensionModuleInfo.module + "." + extensionModuleInfo.roomsEnumName
      );
      final enumInfo = enumType.getEnum();

      for (constuct in enumInfo.constructs) {
        switch (constuct.type) {
          case haxe.macro.Type.TEnum(_, _):
            // Sys.println("    No Args");
            roomFields.push(makeEnumField(constuct.name, FVar(null, null)));
          case haxe.macro.Type.TFun(args, _):
            Sys.println(enumInfo.name);
            Sys.println("  " + constuct.name);
              final functionArgs: Array<FunctionArg> = [];
            for (arg in args) {
              Sys.println("    " + arg.name);
              switch (arg.t) {
                case haxe.macro.Type.TAbstract(argType, _):
                  // Sys.println("      " + argType.get());
                  functionArgs.push({
                    name: "value",
                    opt: false,
                    type: argType.get().type.toComplexType()
                  });
                  break;
                case haxe.macro.Type.TInst(argType, _):
                  Sys.println("      " + argType);
                default:
                  trace("Unexpected enum parameter type: " + arg.t);
              }

              // trace(Type.typeof(Int).toComplexType());
            }
            final eFunc = macro function(value:Int) { };
            // trace(eFunc.expr);
            switch (eFunc.expr) {
              case EFunction(_, f): trace(f.args[0].type);
              default:
            }

            final func: Function = {
              args: functionArgs,
              expr: {
                expr: EBlock([]),
                pos: Context.currentPos()
              },
              params: [],
              ret: null
            };
            roomFields.push(makeEnumField(constuct.name, FFun(func)));
            // trace(func);
            trace(func.args[0].type);
          default:
            trace("Unexpected enum constructor type: " + constuct.type);
        }
      }
    }

    final generatedModule = "game.generated.Generated";
    final roomsEnum: TypeDefinition = {
      fields: roomFields,
      kind: TypeDefKind.TDEnum,
      name: "Rooms",
      pack: generatedModule.split("."),
      pos: Context.currentPos()
    };

    Context.defineModule(generatedModule, [roomsEnum], null, null);
  }

  static public function getImportExpr(module: String): ImportExpr {
    final path: Array<{pos: Position, name: String}> = [];
    final moduleNames = module.split(".");

    for (name in moduleNames) {
      path.push({
        pos: Context.currentPos(),
        name: name
      });
    }

    return {
      path: path,
      mode: INormal
    };
  }

  static function makeEnumField(name, kind): Field {
    return {
      access: [],
      doc: null,
      kind: kind,
      meta: [],
      name: name,
      pos: Context.currentPos()
    }
  }
}
