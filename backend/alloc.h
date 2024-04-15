#ifndef PCGAME_ALLOC_H
#define PCGAME_ALLOC_H

#include <stddef.h>

void *Allocate(size_t size);
void FreeAll(void);

#endif // PCGAME_ALLOC_H
