#ifndef PCGAME_FILELOADING_H
#define PCGAME_FILELOADING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool LoadFile(char *, size_t *, void **, uint16_t, void *);
void UnloadFile(void *, uint16_t, void *);

#endif // PCGAME_FILELOADING_H
