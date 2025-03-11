import haxe.macro.Context;
import haxe.macro.Expr;
import haxe.macro.Type;
using haxe.macro.Tools;
using haxe.io.Path;
using sys.FileSystem;

class GameGeneration {
  static final extensionInfos: Array<{path: String, fieldName: String}> = [{
    path: "backend.coregame.Actions",
    fieldName: "CoreActions"
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
      generateMap("Equipment", 1),
      generateEnum("Rooms", 2),
      generateMap("Screen", 3)
    ];

    Context.defineModule(generatedModule, generatedTypes);
  }

  // TODO: Remove?
  static function generateArray(name: String, idx: Int): TypeDefinition {
    final arrayItemExprs: Array<Expr> = [];

    for (extensionInfo in extensionInfos) {
      final objectExpr: Null<TypedExprDef> = getExtensionObject(extensionInfo, idx);

      var arrayItems: Null<Array<TypedExpr>> = null;
      var objectFieldExpr: FieldAccess;
      switch (objectExpr) {
        case TArrayDecl(el):
          arrayItems = el;
        case TField(_, fa):
          objectFieldExpr = fa;
        default:
          continue;
      }

      if (arrayItems == null) {
        var arrayExpr: TypedExprDef;
        switch (objectFieldExpr) {
          case FStatic(_, cf):
            final arrayClass: ClassField = cf.get();
            arrayExpr = arrayClass.expr().expr;
          default:
            continue;
        }

        switch(arrayExpr) {
          case TArrayDecl(el):
            arrayItems = el;
          default:
            continue;
        }
      }

      for (arrayItem in arrayItems) {
        final arrayItemExpr: Expr = Context.getTypedExpr(arrayItem);
        arrayItemExpr.expr = fixModuleStatic(arrayItemExpr.expr);
        arrayItemExprs.push(arrayItemExpr);
      }
    }

    final arrayExpr: Expr = macro $a{arrayItemExprs};

    return {
      fields: [],
      kind: TDField(FVar(null, arrayExpr)),
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

  static function generateMap(name: String, idx: Int): TypeDefinition {
    final mapItemExprs: Array<Expr> = [];

    for (extensionInfo in extensionInfos) {
      final objectExpr: Null<TypedExprDef> = getExtensionObject(extensionInfo, idx);

      var objectCastExpr: TypedExprDef;
      switch (objectExpr) {
        case TCast(e, _):
          objectCastExpr = e.expr;
        default:
          continue;
      }

      var objectFieldExpr: FieldAccess;
      switch (objectCastExpr) {
        case TField(_, fa):
          objectFieldExpr = fa;
        default:
          continue;
      }

      var mapExpr: TypedExprDef;
      switch (objectFieldExpr) {
        case FStatic(_, cf):
          final mapClass: ClassField = cf.get();
          mapExpr = mapClass.expr().expr;
        default:
          continue;
      }

      var mapItems: Array<TypedExpr>;
      switch (mapExpr) {
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

        var mappingLHSExpr: Expr;
        var mappingRHSExpr: Expr;
        if (mapItemElems.length > 2) {
          continue;
        } else if (mapItemElems.length == 2) {
          switch (mapItemElems[0].expr) {
            case TVar(v, expr):
              mappingRHSExpr = Context.getTypedExpr(expr);
            default:
              continue;
          }

          switch (mapItemElems[1].expr) {
            case TCall(_, el):
              mappingLHSExpr = Context.getTypedExpr(el[0]);
            default:
              continue;
          }
        } else {
          switch (mapItemElems[0].expr) {
            case TCall(_, el):
              mappingLHSExpr = Context.getTypedExpr(el[0]);
              mappingRHSExpr = Context.getTypedExpr(el[1]);
            default:
              continue;
          }
        }

        mappingRHSExpr.expr = fixModuleStatic(mappingRHSExpr.expr);

        mapItemExprs.push({
          expr: EBinop(OpArrow, mappingLHSExpr, mappingRHSExpr),
          pos: Context.currentPos()
        });
      }
    }

    final mapExpr: Expr = macro $a{mapItemExprs};

    return {
      fields: [],
      kind: TDField(FVar(null, mapExpr)),
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
    } catch (e: haxe.macro.Error) {
      if (e.message == "Type not found : " + extensionInfo.path) {
        return null;
      }

      throw e;
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
