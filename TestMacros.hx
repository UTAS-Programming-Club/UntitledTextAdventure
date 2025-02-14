import haxe.macro.Context;
import haxe.macro.Expr;
import haxe.macro.Type;
using haxe.macro.Tools;
using haxe.io.Path;
using sys.FileSystem;

class GameGeneration {
  static final extensionPaths: Array<String> = ["game.Game"];
  static final extensionsDir: String = "extensions";

  static public function generateRoomsEnum(): Void {
    final extensionFiles: Array<String> = extensionsDir.readDirectory();
    for (extensionFile in extensionFiles) {
      if (Path.extension(extensionFile) != "hx") {
        continue;
      }

      final extensionPath: String = Path.join([extensionsDir, extensionFile]);
      final extensionName: String = extensionPath.withoutDirectory().withoutExtension();
      var extensionModule: String = extensionPath.withoutExtension();
      extensionModule = ~/[\\\/]/g.replace(extensionModule, ".");
      extensionPaths.push(extensionModule + "." + extensionName + "Extension");
    }

    Context.onAfterInitMacros(generateRoomsEnumInternal);
  }

  static function generateRoomsEnumInternal(): Void {
    final roomFields: Array<Field> = [];

    for (extensionPath in extensionPaths) {
      var enumInfo;
      final extensionClass: ClassType = Context.getType(extensionPath).getClass();
      final roomsField: Null<ClassField> = extensionClass.findField("rooms");
      final roomsExpr: Null<TypedExprDef> = roomsField?.expr().expr;
      switch (roomsExpr) {
        case TTypeExpr(m):
          switch (m) {
            case TEnumDecl(e):
              enumInfo = e.get();
            default:
              continue;
          }
        default:
         continue;
      }

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
