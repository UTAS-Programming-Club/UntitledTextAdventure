package backend;

enum HealthChangeType {
  PhysicalCombat;
  MagicalCombat;
  Heal;
}

abstract class Entity {
  private static final MaximumStat: UInt = 100;

  public var health(default, null): UInt;
  public var stamina(default, null): UInt;
  public var physicalAttack(default, null): UInt;
  public var physicalDefence(default, null): UInt;
  public var magicalAttack(default, null): UInt;
  public var magicalDefence(default, null): UInt;

  // private var strength: UInt;
  private var agility: UInt;
  // private var wisdom: UInt;
  // private var charisma: UInt;

  public function new(stats: EntityStats) {
    this.health = stats.health;
    this.stamina = stats.stamina;
    this.physicalAttack = stats.physicalAttack;
    this.physicalDefence = stats.physicalDefence;
    this.magicalAttack = stats.magicalAttack;
    this.magicalDefence = stats.magicalDefence;

    // strength = stats.strength;
    this.agility = stats.agility;
    // wisdom = stats.wisdom;
    // charisma = stats.charisma;
  }

  // public function ModifyHealth(type: HealthChangeType, amount: Int): Void {
  //  switch {
  //    case PhysicalCombat:
  //       final newHealth = this.health + this.physicalDefence;
  //       
  //       
  //  }
  //  if(physAtk){
  //    health = health - (amount - physDef);
  //  } else {
  //    health = health - (amount - magDef);
  //  }
  // 
  //   this.health = ModifyStat(this.health, amount);
  // 
  //       if(health < 0){
  //           health = 0;
  //       }
  //   }

  //   public function HealEntity(amount: Int, at DamageType): Void{
  //       health += amount;
  // 
  //       if(health > maxHealth){
  //           health = maxHealth;
  //       }
  //   }
}

class EntityStats {
  public final health: UInt;
  public final stamina: UInt;
  public final physicalAttack: UInt;
  public final physicalDefence: UInt;
  public final magicalAttack: UInt;
  public final magicalDefence: UInt;

  public final agility: UInt;


  public function new(health: UInt, stamina: UInt,
                      physicalAttack: UInt, physicalDefence: UInt,
                      magicalAttack: UInt, magicalDefence: UInt,
                      agility: UInt) {
    this.health = health;
    this.stamina = stamina;
    this.physicalAttack = physicalAttack;
    this.physicalDefence = physicalDefence;
    this.magicalAttack = magicalAttack;
    this.magicalDefence = magicalDefence;
    this.agility = agility;
  }
}
