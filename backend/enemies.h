#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <types.h>

#include "game.h"

struct Enemy{
	// stats here
	bool dead;
	PlayerStat maxHealth;
	PlayerStat currentHealth;
	// 0 physical/1 magic
	bool dmgType;
	PlayerStatDiff damage;
};

void EnemyAttackSequ(struct GameState *, struct Enemy *);

// TODO: Individual attacks enemies can use with status' and stuff idk
// would have attack list with things like descriptor (blasted, stabbed etc.)