import haxe.io.Path;
import haxe.macro.Context;
import haxe.macro.Expr;
import haxe.macro.Printer;
import sys.FileSystem;
import sys.io.File;
using haxe.macro.ComplexTypeTools;
using haxe.macro.TypeTools;

class EnumBuildingMacro {
  static var extensionPaths: Array<String> = [];

  static function test2(): Void {
    var extensionIncludes: Array<ImportExpr> = [];

    for (extensionPath in extensionPaths) {
      var extensionModule = Path.withoutExtension(extensionPath);
      extensionModule = ~/[\\\/]/g.replace(extensionModule, ".");
      final extensionModuleParts = extensionModule.split(".");
      // trace(extensionRoomsEnumName);
      // final contents: String = File.getContent(extensionPath);
      // Sys.println("expected contents: \"" + contents + "\"");

      // final test: TypeDefinition = {
      //   fields: [],
      //   kind: TypeDefKind.TDEnum,
      //   name: extensionRoomsEnumName,
      //   pack: extensionModuleParts,
      //   pos: Context.currentPos()
      // };
      // final printer = new Printer();
      // Sys.println("actual contents: \"" + printer.printTypeDefinition(test, false) + "\"\n");

      extensionIncludes.push({
         path: [{
           pos: Context.currentPos(),
           name: "extensions",
         }, {
           pos: Context.currentPos(),
           name: extensionModuleParts[extensionModuleParts.length - 1],
         },
        ], 
        mode: INormal
      });

      // Context.defineModule("extensions.Extensions", [], [iE], null);
      
      // Sys.println(Context.getType("extensions.Traps"));
      // final enumType: haxe.macro.Type = Context.getType(extensionModule + "." + extensionRoomsEnumName);

      // Sys.println("module: " + extensionModule);
      // Sys.println(enumType);
      // for (constuct in enumType.getEnum().constructs) {
      //   // Sys.println(constuct.name);
      //   // Sys.println(constuct.type);
      //   break;
      // }
      
      // Sys.println(enumType.toComplexType());
      // Sys.println(Type.allEnums(enumType.getEnum()));
    }

    Context.defineModule("extensions.Extensions", [], extensionIncludes, null);

    for (extensionPath in extensionPaths) {
      var extensionModule = Path.withoutExtension(extensionPath);
      extensionModule = ~/[\\\/]/g.replace(extensionModule, ".");
      final extensionModuleParts = extensionModule.split(".");
      final extensionRoomsEnumName = extensionModuleParts[extensionModuleParts.length - 1] + "Rooms";
    
      final enumType: haxe.macro.Type = Context.getType(extensionModule + "." + extensionRoomsEnumName);
      // final t2 = ComplexTypeTools.toType(macro:Enum<Any>);
      final enumInfo = enumType.getEnum();
      
      Sys.println(enumInfo.name);
      // trace(Type.getClassName(enumInfo.constructs));
      for (constuct in enumInfo.constructs) {
        Sys.println("  " + constuct.name);
        switch (constuct.type) {
          case haxe.macro.Type.TEnum(_, _):
            Sys.println("    No Args");
          case haxe.macro.Type.TFun(args, _):
            for (arg in args) {
              Sys.println("    " + arg.name);
              switch (arg.t) {
                case haxe.macro.Type.TAbstract(argType, _):
                  Sys.println("      " + argType);
                default:
              }
            }
          default:
        }
        // for (type in constuct.type) {
        //   Sys.println("    " + type);
        // }
      }
    }

    // final position = Context.getMacroStack()[0];
    // final fileName = Context.getPosInfos(position).file;
    // final moduleName = Path.withoutExtension(fileName);
    // trace(moduleName);

    final roomsEnum: TypeDefinition = {
      fields: [
        makeEnumField("Cool", FVar(null, null))
      ],
      kind: TypeDefKind.TDEnum,
      name: "Rooms",
      pack:  ["game", "Extensions"],
      pos: Context.currentPos()
    };
    // Context.defineType(roomsEnum);
    Context.defineModule("game.Extensions", [roomsEnum], null, null);
  }

  macro static public function test(): haxe.macro.Expr.ExprOf<String> {
    final extensionFiles: Array<String> = FileSystem.readDirectory("extensions");
    for (extensionFile in extensionFiles) {
      if (Path.extension(extensionFile) == "hx") {
        final extensionPath = Path.join(["extensions", extensionFile]);
        extensionPaths.push(extensionPath);
      }
    }

    Context.onAfterInitMacros(test2);
    return macro "hi";
  }

  macro static public function build():Array<Field> {
    var rooms: Array<Field> = [];

    final roomEnums: Array<Enum<Any>> = [CoreRooms];
    for (roomEnum in roomEnums) {
      for (room in Type.allEnums(roomEnum)) {
        rooms.push(makeEnumField(Std.string(room), FVar(null, null)));
      }
    }

    // trace(rooms);

    return rooms;
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

// #if (eval && !macro)
// @:build(EnumBuildingMacro.build())
// #end
// enum Rooms {
// }
