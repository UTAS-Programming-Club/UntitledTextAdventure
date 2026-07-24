#include <errno.h>     // for ERANGE, errno
#include <fcntl.h>     // for O_RDONLY, open
#include <inttypes.h>  // for uint64_t, PRIu64
#include <stdio.h>     // for size_t, fputs, fclose, fprintf, FILE, fopen
#include <stdlib.h>    // for EXIT_FAILURE, realloc, free, EXIT_SUCCESS, strtoull
#include <string.h>    // for memcpy
#include <sys/mman.h>  // for MAP_FAILED, MAP_PRIVATE, PROT_READ, mmap, munmap
#include <sys/stat.h>  // for stat, fstat
#include <uchar.h>     // for char8_t
#include <unistd.h>    // for close


#define DYN_ARRAY(typeName, baseTypeName, varName) struct typeName {                                  \
  baseTypeName *varName ## s;                                                                         \
  size_t count;                                                                                       \
  size_t length;                                                                                      \
};                                                                                                    \
                                                                                                      \
static bool add_ ## varName(struct typeName *const varName ## s, const baseTypeName *const varName) { \
if (varName ## s->count + 1 >= varName ## s->length) {                                                \
    size_t newLen = 2 * varName ## s->count;                                                          \
    if (0 == newLen) {                                                                                \
      newLen = 8;                                                                                     \
    }                                                                                                 \
                                                                                                      \
    baseTypeName *newArr = realloc(varName ## s->varName ## s, newLen * sizeof *varName);             \
    if (nullptr == newArr) {                                                                          \
      fputs("Error: An unrecoverable error occurred\n", stderr);                                      \
      return false;                                                                                   \
    }                                                                                                 \
                                                                                                      \
    varName ## s->length = newLen;                                                                    \
    varName ## s->varName ## s = newArr;                                                              \
  }                                                                                                   \
                                                                                                      \
  memcpy(varName ## s->varName ## s + varName ## s->count, varName, sizeof *varName);                 \
  ++varName ## s->count;                                                                              \
  return true;                                                                                        \
}


enum TokenType {
  IntegerLiteralToken,
  StringLiteralToken,

  ActionTypeToken,
  RoomTypeToken,
  ScreenTypeToken,

  OpenParenToken,
  CloseParenToken,
  SemicolonToken,
  EqualsToken,
  CommaToken,

  IdentifierToken
};

struct Token {
  enum TokenType type;
  union {
    uint64_t integer;     // IntegerLiteralToken
    struct {
      const char8_t *str;
      size_t strLen;
    } string;             // StringLiteralToken, IdentifierToken
  };
};

DYN_ARRAY(TokenInfo, struct Token, token)


enum ExpressionType {
  ActionDefintionExpression,
  RoomDefinitionExpression,
  ScreenDefinitionExpression
};

struct Expression {
  enum ExpressionType type;
  union {
    struct {
      const struct Token *identifier, *string;
    } action;
    struct {
      const struct Token *identifier, *integer1, *integer2, *string;
    } room;
    struct {
      const struct Token *identifier, *string;
    } screen;
  };
};

DYN_ARRAY(ExpressionInfo, struct Expression, expr)


static bool lex_int(const char8_t *str, const char8_t *const expectedEnd, uint64_t *const value) {
  char *end = nullptr;
  *value = strtoull((const char *)str, &end, 10);
  if (ERANGE == errno || (0 == *value && str[0] != '0')) {
    fputs("Error: An unexpected character was encountered in integer literal\n", stderr);
    return false;
  }

  str = (const char8_t *)end;
  return str == expectedEnd;
}

static bool lex(const char8_t *str, struct TokenInfo *const tokens) {
  uint64_t intValue;
  const char8_t *marker = str;
  while (true) {
    const char8_t *previous = str;
    /*!re2c
        re2c:yyfill:enable = 0;
        re2c:sentinel = 0;
        re2c:define:YYCTYPE = char8_t;
        re2c:define:YYCURSOR = str;
        re2c:define:YYMARKER = marker;

        end = "\x00";

        *   {
          fputs("Error: An unxpected character was encountered\n", stderr);
          return false;
        }
        end {
          return true;
        }

        // White space
        wsp = [ \t\v\n\r]+;
        wsp {
          continue;
        }

        // Integer literal
        int = [0-9]*;
        int {
          if (!lex_int(previous, str, &intValue)) {
            return false;
          }
          struct Token token = {
            IntegerLiteralToken,
            .integer = intValue
          };
          add_token(tokens, &token);
          continue;
        }

        // String literal
        str = "\"" [^\x00"]* "\"";
        str {
          struct Token token = {
            StringLiteralToken,
            .string = {
              previous,
              (size_t)(str - previous)
            }
          };
          add_token(tokens, &token);
          continue;
        }

        // Types
        "Action" {
          struct Token token = { ActionTypeToken };
          add_token(tokens, &token);
          continue;
        }
        "Room"   {
          struct Token token = { RoomTypeToken };
          add_token(tokens, &token);
          continue;
        }
        "Screen" {
          struct Token token = { ScreenTypeToken };
          add_token(tokens, &token);
          continue;
        }

        // Symbols
        "(" {
          struct Token token = { OpenParenToken };
          add_token(tokens, &token);
          continue;
        }
        ")" {
          struct Token token = { CloseParenToken };
          add_token(tokens, &token);
          continue;
        }
        ";" {
          struct Token token = { SemicolonToken };
          add_token(tokens, &token);
          continue;
        }
        "=" {
          struct Token token = { EqualsToken };
          add_token(tokens, &token);
          continue;
        }
        "," {
          struct Token token = { CommaToken };
          add_token(tokens, &token);
          continue;
        }

        // Identifier
        id = [^\x00 \t\v\n\r"()=,]+;
        id {
          struct Token token = {
            IdentifierToken,
            .string = { previous, (size_t)(str - previous) }
          };
          add_token(tokens, &token);
          continue;
        }

        // [^=(),"\x00]+ { return TextToken; }
    */
  }
}


#define ADDITIONAL_TOKENS_ERROR() fputs("Error: Additional tokens were expected\n", stderr)
#define UNEXPECTED_TOKEN_ERROR()  fputs("Error: An unxpected token was encountered\n", stderr)
#define SINGLE_PARSE_ALLOW(tokenType) \
  ++token;                            \
  if (token > end) {                  \
    ADDITIONAL_TOKENS_ERROR();        \
    return false;                     \
  }                                   \
                                      \
  switch (token->type) {              \
    case tokenType: break;            \
    default:                          \
      UNEXPECTED_TOKEN_ERROR();       \
      return false;                   \
  }                                   \
                                      \
  token

static bool parse(const struct TokenInfo *const tokens, struct ExpressionInfo *const exprs) {
  const struct Token *token = tokens->tokens;
  const struct Token *const end = token + tokens->count;
  for (; token < end; ++token) {
    switch (token->type) {
      case ActionTypeToken: goto action;
      case RoomTypeToken: goto room;
      case ScreenTypeToken: goto screen;
      default:
        UNEXPECTED_TOKEN_ERROR();
        return false;
    }

action:
    const struct Token *identifier = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(EqualsToken);
    SINGLE_PARSE_ALLOW(ActionTypeToken);
    SINGLE_PARSE_ALLOW(OpenParenToken);
    const struct Token *string = SINGLE_PARSE_ALLOW(StringLiteralToken);
    SINGLE_PARSE_ALLOW(CloseParenToken);

    ++token;
    if (token > end) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }
    switch (token->type) {
      case SemicolonToken:
        struct Expression expr = {
          ActionDefintionExpression,
          .action = { identifier, string }
        };
        add_expr(exprs, &expr);
        continue;
      default:
        UNEXPECTED_TOKEN_ERROR();
        return false;
    }

room:
    identifier = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(EqualsToken);
    SINGLE_PARSE_ALLOW(RoomTypeToken);
    SINGLE_PARSE_ALLOW(OpenParenToken);
    const struct Token *integer1 = SINGLE_PARSE_ALLOW(IntegerLiteralToken);
    SINGLE_PARSE_ALLOW(CommaToken);
    const struct Token *integer2 = SINGLE_PARSE_ALLOW(IntegerLiteralToken);
    SINGLE_PARSE_ALLOW(CommaToken);
    string = SINGLE_PARSE_ALLOW(StringLiteralToken);
    SINGLE_PARSE_ALLOW(CloseParenToken);

    ++token;
    if (token > end) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }
    switch (token->type) {
      case SemicolonToken:
        struct Expression expr = {
          RoomDefinitionExpression,
          .room = { identifier, integer1, integer2, string }
        };
        add_expr(exprs, &expr);
        continue;
      default:
        UNEXPECTED_TOKEN_ERROR();
        return false;
    }

screen:
    identifier = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(EqualsToken);
    SINGLE_PARSE_ALLOW(ScreenTypeToken);
    SINGLE_PARSE_ALLOW(OpenParenToken);
    string = SINGLE_PARSE_ALLOW(StringLiteralToken);
    SINGLE_PARSE_ALLOW(CloseParenToken);

    ++token;
    if (token > end) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }
    switch (token->type) {
      case SemicolonToken:
        struct Expression expr = {
          ScreenDefinitionExpression,
          .screen = { identifier, string }
        };
        add_expr(exprs, &expr);
        continue;
      default:
        UNEXPECTED_TOKEN_ERROR();
        return false;
    }
  }

  return true;
}


struct String {
  const char8_t *str;
  size_t strLen;
};

DYN_ARRAY(StringInfo, struct String, string)

static bool codegen(const struct ExpressionInfo *const exprs, const char *const headerPath, const char *const sourcePath) {
  struct StringInfo rooms = {};

  FILE *const fh = fopen(headerPath, "wb");
  if (nullptr == fh) {
    fprintf(stderr, "Error: unable to open %s\n", headerPath);
    return false;
  }

  fputs("\
#ifndef UTA_EXT1_H\n\
#define UTA_EXT1_H\n\
\n\
#include <stddef.h>\n\
\n\
extern const size_t Ext1_RoomCount;\n\
extern const struct Room *const Ext1_Rooms[];\n\
\n\
#endif // UTA_EXT1_H\n", fh);

  fclose(fh);

  FILE *const fc = fopen(sourcePath, "wb");
  if (nullptr == fc) {
    fprintf(stderr, "Error: unable to open %s\n", sourcePath);
    return false;
  }

  fprintf(fc, "\
#include <stddef.h>\n\
\n\
#include \"backend.h\"\n\
#include \"%s\"\n\n", headerPath);

  for (size_t i = 0; i < exprs->count; ++i) {
    const struct Expression *const expr = exprs->exprs + i;
    switch (expr->type) {
      case ActionDefintionExpression:
      case ScreenDefinitionExpression:
        break;
      case RoomDefinitionExpression:
        struct String room = { expr->room.identifier->string.str,  expr->room.identifier->string.strLen };
        if (!add_string(&rooms, &room)) {
          fclose(fc);
          free(rooms.strings);
          return false;
        }
        fprintf(fc, "const struct Room Ext1_%.*s = NEW_ROOM(%" PRIu64", %" PRIu64", %.*s);\n",
          expr->room.identifier->string.strLen, expr->room.identifier->string.str,
          expr->room.integer1->integer, expr->room.integer2->integer,
          expr->room.string->string.strLen, expr->room.string->string.str
        );
        break;
    }
  }

  fprintf(fc, "\nconst struct Room *const Ext1_Rooms[] = { ");
  for (size_t i = 0; i < rooms.count; ++i) {
    const struct String *const room = rooms.strings + i;
    if (i > 0) {
      fputs(", ", fc);
    }
    fprintf(fc, "&Ext1_%.*s", room->strLen, room->str);
  }
  fputs(" };\n", fc);
  fputs("const size_t Ext1_RoomCount = ARR_COUNT(Ext1_Rooms);\n", fc);

  fclose(fc);
  free(rooms.strings);
  return true;
}


#define USAGE "Usage: %s input output_header output_source\n"
#define PATH_CHECK(name, idx, expectedExt, error)                             \
  const char *const name = argv[idx];                                         \
  ext = strstr(name, expectedExt);                                            \
  if (nullptr == ext || name == ext || '\0' != ext[sizeof expectedExt - 1]) { \
    fprintf(stderr, USAGE, argv[0]);                                          \
    fputs("Error: " error "\n", stderr);                                      \
    return EXIT_FAILURE;                                                      \
  }

int main(const int argc, const char *const argv[argc]) {
  bool status = true;
  if (1 == argc || (2 == argc && 0 == strcmp(argv[1], "-h"))) {
    printf(USAGE, argv[0]);
    return EXIT_SUCCESS;
  }

  if (4 != argc) {
    fprintf(stderr, USAGE, argv[0]);
    return EXIT_FAILURE;
  }

  const char *ext;
  PATH_CHECK(inputPath,   1, ".uta", "input must be a uta source file");
  PATH_CHECK(outputHPath, 2,   ".h", "output header must be a c header file");
  PATH_CHECK(outputCPath, 3,   ".c", "output source must be a c source file");

  int fd = open(inputPath, O_RDONLY);
  if (-1 == fd) {
    fprintf(stderr, "Error: unable to open %s\n", inputPath);
    return EXIT_FAILURE;
  }

  struct stat st;
  if (-1 == fstat(fd, &st)) {
    fprintf(stderr, "Error: unable to process %s\n", inputPath);
    return EXIT_FAILURE;
  }

  void *file =  mmap(nullptr, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  status &= MAP_FAILED != file;

  struct TokenInfo tokens = {};
  status = status && lex(file, &tokens);

  struct ExpressionInfo exprs = {};
  status = status && parse(&tokens, &exprs);

  status = status && codegen(&exprs, outputHPath, outputCPath);

  free(exprs.exprs);
  free(tokens.tokens);

  munmap(file, (size_t)st.st_size);
  close(fd);

  return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
