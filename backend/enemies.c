#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "enemies.h"
#include "equipment.h"
#include "game.h"

void EnemyAttackSequ(struct GameState *state, struct Enemy *enemy)
{
  // TODO: loops throuhg enemy array and takes their actions
  // TODO: choses random attack
  // TODO: if attack is physical/magical pass in as a bool
  
  if(PlayerTakeDamage(&state->playerInfo, enemy->damage)){
    // TODO: make enemy attacks and events in combat modify string
    
  }
  else{
    // changes outcome if dodged
  }
}

char *CreateCombatString(const struct GameState *state, struct Enemy *enemy){
  return "c";
}