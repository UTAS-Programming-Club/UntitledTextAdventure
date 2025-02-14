import haxe.macro.Context;
import haxe.macro.Expr;
import haxe.macro.Type;
using haxe.macro.Tools;
using haxe.io.Path;
using sys.FileSystem;

class GameGeneration {
  static final extensionInfos: Array<{path: String, fieldName: String}> = [{
    path: "game.Game",
    fieldName: "Game"
  }];
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
      extensionInfos.push({
        path: extensionModule,
        fieldName: extensionName + "Extension"
      });
    }

    Context.onAfterInitMacros(generateRoomsEnumInternal);
  }

  static function generateRoomsEnumInternal(): Void {
    final roomFields: Array<Field> = [];

    for (extensionInfo in extensionInfos) {
      var moduleTypes: Array<Type>;
      try {
        moduleTypes = Context.getModule(extensionInfo.path);
      } catch (e: haxe.macro.Error) {
        continue;
      }

      var moduleClass: ClassType;
      for (type in moduleTypes) {
        final moduleType: ModuleType = type.toModuleType();
        switch (moduleType) {
          case TClassDecl(c):
            moduleClass = c.get();
            break;
          default:
            continue;
        }
      }

      var gameFieldExpr: Null<TypedExprDef>;
      switch (moduleClass?.kind) {
        case KModuleFields(_):
          final gameField: Null<ClassField> = moduleClass.findField(
            extensionInfo.fieldName, true
          );
          gameFieldExpr = gameField?.expr().expr;
        default:
          continue;
      }

      var enumInfo;
      switch (gameFieldExpr) {
        case TNew(_, _, [e]):
          switch (e.expr) {
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
