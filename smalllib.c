#include "smalllib.h"

void write(int stream, const void* buf, size_t len) {
	__asm__ (
		"movq $1, %%rax\n\t" // write
		"movq %0, %%rdi\n\t" // fd
		"movq %1, %%rsi\n\t" // buf
		"movq %2, %%rdx\n\t" // count
		"syscall"
		: /* no out */
		: "g" ((unsigned long long int)stream), "g" (buf), "g" (len)
		: "%rax", "%rdi", "%rsi", "%rdx"
	);
}

size_t strlen(const char *str)
{
	size_t i = 0;
	while(1) {
		if(str[i] == '\0')
			return i;
		++i;
	}

	// note - unreachable
	return 0;
}

void fputs(const char *str, int stream)
{
	write(stream, str, strlen(str));
}

void puts(const char *str)
{
	fputs(str, stdout);
	putchar('\n');
}

void putchar(const char ch)
{
	write(stdout, &ch, 1);
}

int scant(size_t size, void *inp)
{
	__asm__ (
		"movq $0, %%rax\n\t" // write
		"movq %1, %%rdi\n\t" // fd
		"movq %0, %%rsi\n\t" // buf (out)
		"movq %2, %%rdx\n\t" // size
		"syscall"
		: /* no output (well in this way) */
		: "g" (inp), "g" (stdin), "g" (size)
		: "%rax", "%rdi", "%rsi", "%rdx"
	);

	// assume some input validation
	return 1;
}

__attribute__ ((__optimize__ ("-fno-tree-loop-distribute-patterns"))) void memset(void *dest, int ch, size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		((char *)dest)[i] = ch;
	}
}


void printuint(unsigned long long val) {
  char buf[22]; // 21 chars is max for 64 bit int
  buf[21] = '\0';

  size_t digitCount = 0;
  do
  {
      buf[20 - digitCount] = '0' + val % 10;
      val /= 10;
      ++digitCount;
  }
  while (val > 0);

  fputs(buf + 21 - digitCount, stdout);
}

void printint(long long val) {
  if (val < 0) {
    putchar('-');
    val = -val;
  }

  printuint(val);
}

