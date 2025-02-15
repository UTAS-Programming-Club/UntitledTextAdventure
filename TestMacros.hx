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
  static final generatedModule: String = "game.generated.Generated";

  static public function generateEnums(): Void {
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

    Context.onAfterInitMacros(generateEnumsInternal);
  }

  static function generateEnumsInternal(): Void {
    final generatedEnums: Array<TypeDefinition> = [
      generateEnum("Rooms", 0),
      generateEnum("Actions", 1)
    ];

    Context.defineModule(generatedModule, generatedEnums);
  }

  static function generateEnum(name: String, idx: Int): TypeDefinition {
    final enumFields: Array<Field> = [];

    for (extensionInfo in extensionInfos) {
      var moduleTypes: Array<Type>;
      try {
        moduleTypes = Context.getModule(extensionInfo.path);
      } catch (e: haxe.macro.Error) {
        continue;
      }

      var gameFieldExpr: Null<TypedExprDef>;
      for (type in moduleTypes) {
        var moduleClass: ClassType;
        final moduleType: ModuleType = type.toModuleType();
        switch (moduleType) {
          case TClassDecl(c):
            moduleClass = c.get();
          default:
            continue;
        }

        switch (moduleClass?.kind) {
          case KModuleFields(_):
            final gameField: Null<ClassField> = moduleClass.findField(
              extensionInfo.fieldName, true
            );
            gameFieldExpr = gameField?.expr().expr;
            break;
          default:
            continue;
        }
      }

      var enumInfo;
      switch (gameFieldExpr) {
        case TNew(_, _, e):
          switch (e[idx].expr) {
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
            enumFields.push(makeEnumField(construct.name, FVar(null)));
          case haxe.macro.Type.TFun(args, _):
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

    return {
      fields: enumFields,
      kind: TypeDefKind.TDEnum,
      name: name,
      pack: generatedModule.split("."),
      pos: Context.currentPos()
    };
  }

  static function makeEnumField(name: String, kind: FieldType): Field return {
    kind: kind,
    name: name,
    pos: Context.currentPos()
  }
}
