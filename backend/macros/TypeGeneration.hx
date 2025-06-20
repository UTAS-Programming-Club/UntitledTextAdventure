package backend.macros;

import haxe.macro.Expr;
import haxe.macro.Type;

using haxe.io.Path;
using haxe.macro.Context;
using haxe.macro.ExprTools;
using StringTools;
using sys.FileSystem;

class TypeGeneration {
  static var foundFilePaths: Bool = false;
  static var filePaths: Map<String, Array<String>> = [];

  static function findFilePaths(directory: String = "."): Void {
    if (!directory.exists()) {
      return;
    }

    final files: Array<String> = directory.readDirectory();
    var file: String;
    for (file in files) {
      final path: String = Path.join([directory, file]);

      if (path.isDirectory()) {
        findFilePaths(path);
        continue;
      }

      if (!file.endsWith(".hx")) {
        continue;
      }

      if (filePaths[file] == null) {
        filePaths[file] = [];
      }

      final module: String = path.withoutExtension().replace("/", ".");
      filePaths[file].push(module);
    }

    foundFilePaths = true;
  }
  
  static public function buildGameEnum(fileName: String): Array<Field> {
    final enumFields: Array<Field> = [];

    if (!foundFilePaths) {
      findFilePaths();
    }

    if (filePaths[fileName] == null) {
      return enumFields;
    }

    for (enumPath in filePaths[fileName]) {
      final types: Array<Type> = enumPath.getModule();

      // TODO: Check type name
      var enumType: Null<EnumType> = null;
      for (type in types) {
        switch (type) {
          case TEnum(e, _):
            enumType = e.get();
          default:
            continue;
        }
      }

      if (enumType == null) {
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

    return enumFields;
  }

  // NOTE: To create buildGameArray, items are in a TArrayDecl instead of TBlock
  // and each is equivalent to mappingRHSExpr without needing to trim `{` & `}`
  static public function buildGameMap(fileName: String, typeName: String): Array<Field> {
    final fields: Array<Field> = Context.getBuildFields();
    final mapExprs: Array<Expr> = [];

    if (!foundFilePaths) {
      findFilePaths();
    }

    if (filePaths[fileName] == null) {
      return fields;
    }

    var mapField: Null<Field> = null;
    for (field in fields) {
      if (field.name == typeName) {
        mapField = field;
      }
    }

    if (mapField == null) {
      return fields;
    }

    for (mapPath in filePaths[fileName]) {
      final types: Array<Type> = mapPath.getModule();

      // TODO: Check type name
      var classType: Null<ClassType> = null;
      for (type in types) {
        switch (type) {
          case TInst(t, _):
            classType = t.get();
          default:
            continue;
        }
      }

      if (classType == null) {
        continue;
      }

      // Previously, we assumed there was only one static field per extension
      // module but module scoped functions count as statics so now we assume
      // only one static field's name ends with the type of map being created.
      final fields: Array<ClassField> = classType.statics.get();
      var mapExpr: Null<TypedExpr> = null;
      for (field in fields) {
        if (field.name.endsWith(typeName)) {
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

        var mappingLHSExpr: Expr;
        var mappingRHSExpr: Expr;
        if (mapItemElems.length == 0 || mapItemElems.length > 2) {
          continue;
        } else if (mapItemElems.length == 2) {
          switch (mapItemElems[0].expr) {
            case TVar(_, expr):
              mappingRHSExpr = expr.getTypedExpr();
            default:
               continue;
          }

          switch (mapItemElems[1].expr) {
            case TCall(_, el) if (el.length == 2):
              mappingLHSExpr = el[0].getTypedExpr();
            default:
              continue;
          }
        } else {
          switch (mapItemElems[0].expr) {
            case TCall(_, el) if (el.length == 2):
              mappingLHSExpr = el[0].getTypedExpr();
              mappingRHSExpr = el[1].getTypedExpr();
            default:
              continue;
          }
        }

        mappingLHSExpr = fixModuleStatics(mappingLHSExpr);
        mappingRHSExpr = fixModuleStatics(mappingRHSExpr);
        mapExprs.push(
          macro $mappingLHSExpr => $mappingRHSExpr
        );
      }
    }

    switch (mapField.kind) {
      case FVar(t, _):
        mapField.kind = FVar(t, macro $a{mapExprs});
      default:
    }

    return fields;
  }

  static public function GetCampaignObject(): TypedExpr {
    final campaignName: String = Context.definedValue('campaign');
    final campaignPath: String = Path.join(['campaigns', campaignName + '.hx']);
    if (!campaignPath.exists()) {
      throw 'Unable to find campaign: $campaignName.';
    }

    final campaignModule: String = campaignPath.withoutExtension().replace('/', '.');
    final campaignTypes: Array<Type> =  campaignModule.getModule();

    for (type in campaignTypes) {
      var campaignStatics: Null<Array<ClassField>> = null;
      switch (type) {
        case TInst(t, _):
          final possibleClass = t.get();
          if (possibleClass.name != '${campaignName}_Fields_') {
            continue;
          }

          campaignStatics = possibleClass.statics.get();
        default:
          continue;
      }

      if (campaignStatics == null) {
        continue;
      }

      for (field in campaignStatics) {
        final campaignExpr: Null<TypedExpr> = field.expr();
        if (campaignExpr == null) {
          continue;
        }

        switch (campaignExpr.t) {
          case TType(t, _) if (t.toString() == 'backend.Campaign'):
            return campaignExpr;
          default:
            continue;
        }
      }
    }

    throw 'Unable to find campaign: $campaignName.';
  }

  static public function GetExtensions(campaign: TypedExpr): Array<TypedExpr> {
    final campaignName: String = Context.definedValue('campaign');

    var extensions: Null<Array<TypedExpr>> = null;

    var campaignExpr: Null<TypedExpr> = null;
    switch (campaign.expr) {
      case TCast(e, _):
        campaignExpr = e;
      default:
        throw 'Unable to find extensions for $campaignName.';
    }

    var campaignFields: Null<Array<{name: String, expr: TypedExpr}>> = null;
    switch (campaignExpr.expr) {
      case TObjectDecl(fields):
        campaignFields = fields;
      default:
        throw 'Unable to find extensions for $campaignName.';
    }

    for (field in campaignFields) {
      if (field.name != 'extensions') {
        continue;
      }

      switch (field.expr.expr) {
        case TArrayDecl(el):
          return el;
        default:
          continue;
      }
    }

    throw 'Unable to find extensions for $campaignName.';
  }

  static public function macro3(): Array<Field> {
    final fields: Array<Field> = Context.getBuildFields();
    final campaign: TypedExpr = GetCampaignObject();
    final extensions: Array<TypedExpr> = GetExtensions(campaign);

    for (extension in extensions) {
      // var extensionModuleExpr: Null<TypedExprDef> = null;
      var extensionModuleField: Null<FieldAccess> = null;
      switch (extension.expr) {
        case TField(e, fa):
          // extensionModuleExpr = e.expr;
          extensionModuleField = fa;
        default:
          continue;
      }

      var extensionField: Null<TypedExpr> = null;
      switch(extensionModuleField) {
        case FStatic(_, cf):
          extensionField = cf.get().expr();
        default:
          continue;
      }

      if (extensionField == null) {
        continue;
      }

      trace(extension);

      /*var extensionModuleDecl: Null<ModuleType> = null;
      switch (extensionModuleExpr) {
        case TTypeExpr(m):
          extensionModuleDecl = m;
        default:
          continue;
      }

      var extensionStatics: Null<Array<ClassField>> = null;
      switch (extensionModuleDecl) {
        case TClassDecl(c):
          extensionStatics = c.get().statics.get();
        default:
          continue;
      }*/
    }

    return fields;
  }


  static function makeEnumField(name: String, kind: FieldType): Field return {
    kind: kind,
    name: name,
    pos: Context.currentPos()
  }

  static function fixModuleStatics(expr: Expr): Expr {
    return switch (expr.expr) {
      case EField(e, field, _) if (field.endsWith("_Fields_")):
        e;
      default:
        expr.map(fixModuleStatics);
    }
  }
}
