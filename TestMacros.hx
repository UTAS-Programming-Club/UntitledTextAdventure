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

  static function generateTypes(): Void {
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
    var arrayType: Null<ComplexType> = null;
    final arrayItemExprs: Array<Expr> = [];

    for (extensionInfo in extensionInfos) {
      final objectExpr: Null<TypedExprDef> = getExtensionObject(extensionInfo, idx);

      var objectFieldExpr: FieldAccess;
      switch (objectExpr) {
        case TField(_, fa):
          objectFieldExpr = fa;
        default:
          continue;
      }

      var arrayExpr: TypedExprDef;
      switch (objectFieldExpr) {
        case FStatic(_, cf):
          final arrayClass: ClassField = cf.get();
          // TODO: Figure out why (macro : Array<game.EquipmentInfo>) causes "Type not found : game.EquipmentInfo" during Context.defineModule
          arrayType ??= arrayClass.type.toComplexType();
          arrayExpr = arrayClass.expr().expr;
        default:
          continue;
      }

      var arrayItems: Array<TypedExpr>;
      switch(arrayExpr) {
        case TArrayDecl(el):
          arrayItems = el;
        default:
          continue;
      }

      for (arrayItem in arrayItems) {
        final arrayItemExpr: Expr = Context.getTypedExpr(arrayItem);

        switch (arrayItemExpr.expr) {
          case EField(e1, field1, kind1):
            switch (e1.expr) {
              case EField(e2, field2, _):
                switch (e2.expr) {
                  case EField(_, field3, _):
                    final moduleStaticsClassName: String = field3 + "_Fields_";
                    if (moduleStaticsClassName != field2) {
                      continue;
                    }

                    arrayItemExpr.expr = EField(e2, field1, kind1);
                  default:
                }
              default:
            }
          default:
        }

        arrayItemExprs.push(arrayItemExpr);
      }
    }

    final arrayExpr: Expr = macro $a{arrayItemExprs};

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
      final objectExpr: Null<TypedExprDef> = getExtensionObject(extensionInfo, idx);

      var objectCastExpr: TypedExprDef;
      switch (objectExpr) {
        case TCast(e, _):
          objectCastExpr = e.expr;
        default:
          continue;
      }

      var objectModuleType: ModuleType;
      switch (objectCastExpr) {
        case TTypeExpr(m):
          objectModuleType = m;
        default:
          continue;
      }

      var enumType: EnumType;
      switch (objectModuleType) {
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

    return {
      fields: enumFields,
      kind: TDEnum,
      name: name,
      pack: generatedModule.split("."),
      pos: Context.currentPos()
    };
  }

  static function getExtensionObject(
    extensionInfo: {path: String, fieldName: String}, idx: Int
  ): Null<TypedExprDef> {
    var moduleTypes: Array<Type>;
    try {
      moduleTypes = Context.getModule(extensionInfo.path);
    } catch (e: Error) {
      return null;
    }

    var fieldExpr: Null<TypedExprDef> = null;
    for (type in moduleTypes) {
      final moduleType: ModuleType = type.toModuleType();

      var moduleClass: ClassType;
      switch (moduleType) {
        case TClassDecl(c):
          moduleClass = c.get();
        default:
          continue;
      }

      switch (moduleClass.kind) {
        case KModuleFields(_):
          final field: Null<ClassField> = moduleClass.findField(
            extensionInfo.fieldName, true
          );
          fieldExpr = field?.expr().expr;
          break;
        default:
          continue;
      }
    }

    var fieldCastExpr: TypedExprDef;
    switch (fieldExpr) {
      case TCast(e, _):
        fieldCastExpr = e.expr;
      default:
        return null;
    }

    switch (fieldCastExpr) {
      case TObjectDecl(fields):
        return fields[idx].expr.expr;
      default:
        return null;
    }
  }

  static function makeEnumField(name: String, kind: FieldType): Field return {
    kind: kind,
    name: name,
    pos: Context.currentPos()
  }
}
