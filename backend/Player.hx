package backend;

@:nullSafety(Strict)
class Player {
  // private var helmets = List<Equipment>();
  // private var chest = List<Equipment>();
  // private var gloves = List<Equipment>();
  // private var pants = List<Equipment>();
  // private var boots = List<Equipment>();
  // private var primary = List<Equipment>();
  // private var secondary = List<Equipment>();

  // public function SwapEquipment(): Void{
  // }

  public var health:  UInt = 100;
  public var agility: UInt = 20;

  public var X: UInt = 0;
  public var Y: UInt = 0;

  public function new() {
  }
}
