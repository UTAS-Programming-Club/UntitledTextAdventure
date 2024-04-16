#ifndef PCGAME_STRINGS_H
#define PCGAME_STRINGS_H

#include <uchar.h>

int codeunitcount32(const char32_t *);
size_t cpc32rtomb(char *restrict, char32_t);
size_t cpmbrtoc32(char32_t *restrict, const char *restrict, size_t);

#endif // PCGAME_STRINGS_H
