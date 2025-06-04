package backend;

class Player {
  public var health(default, null): Int = 0;
  public var stamina(default, null): Int = 0;

  public function new() {
  }

  public function Reset(): Void {
    health = 100;
    stamina = 100;
  }

  public function ModifyHealth(change: Int): Void {
     var newHealth: Int = health + change;
     if (newHealth < 0) {
      newHealth = 0;
     } else if (newHealth > 100) {
      newHealth = 100;
     }

     health = newHealth;
  }
}
