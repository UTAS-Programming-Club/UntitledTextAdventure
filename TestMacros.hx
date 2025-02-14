import haxe.macro.Context;
import haxe.macro.Expr;
using haxe.macro.Tools;
using haxe.io.Path;
using sys.FileSystem;

class GameGeneration {
  static final extensionEnumPaths: Array<String> = ["game.Rooms"];
  static final extensionRoomsDir: String = "extensions";

  static public function generateRoomsEnum(): Void {
    final extensionFiles: Array<String> = extensionRoomsDir.readDirectory();
    for (extensionFile in extensionFiles) {
      if (Path.extension(extensionFile) != "hx") {
        continue;
      }

      final extensionPath: String = Path.join([extensionRoomsDir, extensionFile]);
      final extensionName: String = extensionPath.withoutDirectory().withoutExtension();
      var extensionModule: String = extensionPath.withoutExtension();
      extensionModule = ~/[\\\/]/g.replace(extensionModule, ".");
      extensionEnumPaths.push(extensionModule + "." + extensionName + "Rooms");
    }

    Context.onAfterInitMacros(generateRoomsEnumInternal);
  }

  static function generateRoomsEnumInternal(): Void {
    final roomFields: Array<Field> = [];

    for (extensionEnumPath in extensionEnumPaths) {
      final enumType: haxe.macro.Type = Context.getType(extensionEnumPath);
      final enumInfo = enumType.getEnum();

      for (construct in enumInfo.constructs) {
        switch (construct.type) {
          case haxe.macro.Type.TEnum(_, _):
            roomFields.push(makeEnumField(construct.name, FVar(null)));
          case haxe.macro.Type.TFun(args, _):
            final func: Function = {
              args: args.map(
                function(arg): FunctionArg return {
                  name: arg.name,
                  type: arg.t.toComplexType()
                }
              )
            };

            roomFields.push(makeEnumField(construct.name, FFun(func)));
          default:
            trace("Unexpected enum constructor type: " + construct.type);
        }
      }
    }

    final generatedModule: String = "game.generated.Generated";
    final roomsEnum: TypeDefinition = {
      fields: roomFields,
      kind: TypeDefKind.TDEnum,
      name: "Rooms",
      pack: generatedModule.split("."),
      pos: Context.currentPos()
    };

    Context.defineModule(generatedModule, [roomsEnum]);
  }

  static function makeEnumField(name: String, kind: FieldType): Field return {
    kind: kind,
    name: name,
    pos: Context.currentPos()
  }
}
