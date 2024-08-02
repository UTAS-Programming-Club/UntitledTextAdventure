#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <types.h>

#include "game.h"

struct Enemy{
	// stats here
	bool dead;
	uint_fast8_t maxHealth
	uint_fast8_t currentHealth;
	// 0 physical/1 magic
	bool dmgType;
	uint_fast8_t damage;
}

string EnemyAttackSequ(struct GameState *, struct Enemy *);

// TODO: Individual attacks enemies can use with status' and stuff idk
// would have attack list with things like descriptor (blasted, stabbed etc.)