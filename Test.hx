import game.generated.Generated;

// haxe --macro "TestMacros.GameGeneration.generateTypes()" --main Test --interp
// haxe --macro "TestMacros.GameGeneration.generateTypes()" --main Test --python test.py && python test.py

// This should give:
// [QuitGame,GoWest,GoSouth,GoNorth,GoEast,OpenChest,DodgeTrap]
// [ChestRags => {name: Rags, type: Chest}, LegRags => {name: Rags, type: Legs}]
// [Empty,Chest,Trap]
class Test {
  public static function main(): Void {
    Sys.println(Type.getEnumConstructs(Actions));
    Sys.println(Equipment.toString());
    Sys.println(Type.getEnumConstructs(Rooms));
  }
}
