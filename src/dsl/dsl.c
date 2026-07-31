#include <fcntl.h>     // for O_RDONLY, open
#include <stdio.h>     // for fprintf, stderr, size_t, printf
#include <stdlib.h>    // for EXIT_FAILURE, free, EXIT_SUCCESS
#include <string.h>    // for strstr, strcmp
#include <sys/mman.h>  // for MAP_FAILED, MAP_PRIVATE, PROT_READ, mmap, munmap
#include <sys/stat.h>  // for stat, fstat
#include <unistd.h>    // for close

#include "dsl/dsl.h"   // IWYU pragma: associated

const char *programName;
const char *inputPath;


#define USAGE "Usage: %s input output_header output_source extension_name\n"
#define PATH_CHECK(idx, expectedExt, error)                                   \
  path = argv[idx];                                                           \
  ext = strstr(path, expectedExt);                                            \
  if (nullptr == ext || path == ext || '\0' != ext[sizeof expectedExt - 1]) { \
    EMIT_PROG_ERROR(error);                                                   \
    fprintf(stderr, USAGE, argv[0]);                                          \
    return EXIT_FAILURE;                                                      \
  }

int main(const int argc, const char *const argv[const restrict static argc]) {
  programName = argv[0];

  bool status = true;
  if (1 == argc || (2 == argc && 0 == strcmp(argv[1], "-h"))) {
    printf(USAGE, argv[0]);
    return 1 == argc ? EXIT_FAILURE : EXIT_SUCCESS;
  }

  if (5 != argc) {
    fprintf(stderr, USAGE, argv[0]);
    return EXIT_FAILURE;
  }

  const char *path, *ext;
  inputPath                     = PATH_CHECK(1, ".uta", "input must be a uta source file");
  const char *const outputHPath = PATH_CHECK(2,   ".h", "output header must be a c header file");
  const char *const outputCPath = PATH_CHECK(3,   ".c", "output source must be a c source file");
  const char *const extensionName = argv[4];

  int fd = open(inputPath, O_RDONLY);
  if (-1 == fd) {
    EMIT_PROG_ERROR("unable to open %s", inputPath);
    return EXIT_FAILURE;
  }

  struct stat st;
  if (-1 == fstat(fd, &st)) {
    status = false;
    EMIT_PROG_ERROR("unable to process %s", inputPath);
  }

  void *file = MAP_FAILED;
  if (status) {
    file = mmap(nullptr, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == file) {
      status = false;
      EMIT_PROG_ERROR("unable to read from %s", inputPath);
    }
  }

  struct TokenInfo tokens = {};
  status = status && lex(file, &tokens);

  struct ExpressionInfo exprs = {};
  status = status && parse(&tokens, &exprs);

  status = status && codegen(&exprs, outputHPath, outputCPath, extensionName);

  for (size_t i = 0; i < exprs.count; ++i) {
    const struct Expression *const expr = exprs.exprs + i;
    switch (expr->type) {
      case EnumDefinitionExpression:
        free(expr->idValues.tokens);
        break;
      case ScreenDefinitionExpression:
        free(expr->screen.idActions.tokens);
        break;
      default:
        break;
    }
  }

  free(exprs.exprs);
  free(tokens.tokens);

  munmap(file, (size_t)st.st_size);
  close(fd);

  return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
