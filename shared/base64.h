#ifndef PCGAME_BASE64_H
#define PCGAME_BASE64_H

#include <stdbool.h>
#include <stddef.h>
#include <uchar.h>

#ifdef PREPTEXT
// Used by the preptext tool
// Must free resulting string
bool c8toc32base64(char *, char **);
#elif BACKEND
// Used by the backend to decode base 64 strings from json
// Must free resulting string
char32_t *c32base64toc32(char *);
#endif

#endif // PCGAME_BASE64_H
