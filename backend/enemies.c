#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "game.h"

string EnemyAttackSequ(struct GameState *state, struct Enemy *enemy)
{
	if(&enemy.dmgType == 0)
	{
		UpdatePlayerStat(&state->playerInfo.health, (&enemy.damage - &state->playerInfo.physDef));
	}
	else
	{
		
	}
	
	return outcome;
}