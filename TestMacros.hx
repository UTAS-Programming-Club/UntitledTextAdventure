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

  static public function generateTypes(): Void {
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

    Context.onAfterInitMacros(generateTypesInternal);
  }

  static function generateTypesInternal(): Void {
    final generatedTypes: Array<TypeDefinition> = [
      generateEnum("Actions", 0),
      generateArray("Equipment", 1),
      generateEnum("Rooms", 2)
    ];

    Context.defineModule(generatedModule, generatedTypes);
  }

  static function generateArray(name: String, idx: Int): TypeDefinition {
    final arrayExprs: Array<Expr> = [];
    var arrayType: Null<ComplexType> = null;

    for (extensionInfo in extensionInfos) {
      var moduleTypes: Array<Type>;
      try {
        moduleTypes = Context.getModule(extensionInfo.path);
      } catch (e: Error) {
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

      var arrayInfo: TypedExprDef;
      switch (gameFieldExpr) {
        case TNew(_, _, e):
          switch (e[idx].expr) {
            case TField(_, fa):
              switch (fa) {
                case FStatic(_, cf):
                  final arrayClass: ClassField = cf.get();
                  // TODO: Figure out why (macro : Array<game.EquipmentInfo>) causes "Type not found : game.EquipmentInfo" during Context.defineModule
                  arrayType ??= arrayClass.type.toComplexType();
                  arrayInfo = arrayClass.expr().expr;
                default:
                  continue;
              }
            default:
              continue;
          }
        default:
          continue;
      }

      var equipmentItems: Array<TypedExpr>;
      switch(arrayInfo) {
        case TArrayDecl(el):
          equipmentItems = el;
        default:
          continue;
      }

      for (equipmentItem in equipmentItems) {
        final expr: Expr = Context.getTypedExpr(equipmentItem);
        switch (expr.expr) {
          case EField(e1, field1, kind1):
            switch (e1.expr) {
              case EField(e2, field2, _):
                switch (e2.expr) {
                  case EField(_, field3, _):
                    final moduleStaticsClass: String = field3 + "_Fields_";
                    if (moduleStaticsClass != field2) {
                      continue;
                    }

                    expr.expr = EField(
                      {
                        expr: e2.expr,
                        pos: e1.pos
                      },
                      field1,
                      kind1
                    );
                  default:
                }
              default:
            }
          default:
        }

        arrayExprs.push(expr);
      }
    }

    final arrayExpr: Expr = macro $a{arrayExprs};

    return {
      fields: [],
      kind: TDField(FVar(arrayType, arrayExpr)),
      name: name,
      pack: generatedModule.split("."),
      pos: Context.currentPos()
    };
  }

  static function generateEnum(name: String, idx: Int): TypeDefinition {
    final enumFields: Array<Field> = [];

    for (extensionInfo in extensionInfos) {
      var moduleTypes: Array<Type>;
      try {
        moduleTypes = Context.getModule(extensionInfo.path);
      } catch (e: Error) {
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

    return {
      fields: enumFields,
      kind: TDEnum,
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
