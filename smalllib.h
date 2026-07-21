#ifndef UTA_SMALLLIB_H
#define UTA_SMALLLIB_H

typedef __UINT8_TYPE__ uint8_t;
typedef __SIZE_TYPE__ size_t;

#define NULL nullptr

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1 // recall this from previous testing

#define stdin 0
#define stdout 1
#define stderr 2

size_t strlen(const char *str);

void fputs(const char *str, int stream);

void puts(const char *str);

void putchar(const char ch);

int scant(size_t size, void *inp);

void memset(void *dest, int ch, size_t count);

void printuint(unsigned long long val);
void printint(long long val);

#endif
