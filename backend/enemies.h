#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <types.h>

#include "game.h"

struct Enemy {
  // stats here
  // bool dead;
  EntityStat health;
  // 0 physical/1 magic
  // bool dmgType;
  // TODO: Use weapon?
  EntityStatDiff damage;
};

void EnemyPerformAttack(struct GameState *, const struct Enemy *);
const char *CreateCombatString(struct GameState *, const struct Enemy *);

// TODO: Individual attacks enemies can use with status' and stuff idk
// would have attack list with things like descriptor (blasted, stabbed etc.)
