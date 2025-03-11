package backend;

@:build(backend.Test.TypeBuildingMacro.generateEnum("Screen", 3))
enum Actions {}

// class Game {
//   static public function main() {
//     trace(Type.getEnumConstructs(Screens)); // my default
//   }
// }
