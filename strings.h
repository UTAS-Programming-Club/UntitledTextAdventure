#ifndef PCGAME_STRINGS_H
#define PCGAME_STRINGS_H

#include <uchar.h>

int codeunitcount32(const char32_t *str);
size_t cpc32rtomb(char *__restrict__ s, char32_t c32);

#endif // PCGAME_STRINGS_H
