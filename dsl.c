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

struct TokenInfo {
  struct Token *tokens;
  size_t count;  // Stored tokens count in items
  size_t length; // Dynamic array length in items
};

static bool add_token(struct TokenInfo *tokens, const struct Token *token) {
  if (tokens->count + 1 >= tokens->length) {
    size_t newLength = 2 * tokens->count;
    if (0 == newLength) {
      newLength = 8;
    }

    struct Token *newTokens = realloc(tokens->tokens, newLength * sizeof *tokens->tokens);
    if (nullptr == newTokens) {
      return false;
    }

    tokens->length = newLength;
    tokens->tokens = newTokens;
  }

  memcpy(tokens->tokens + tokens->count, token, sizeof *token);
  ++tokens->count;
  return true;
}


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

struct ExpressionInfo {
  struct Expression *exprs;
  size_t count;  // Stored exprs count in items
  size_t length; // Dynamic array length in items
};

static bool add_expr(struct ExpressionInfo *exprs, const struct Expression *expr) {
  if (exprs->count + 1 >= exprs->length) {
    size_t newLength = 2 * exprs->count;
    if (0 == newLength) {
      newLength = 8;
    }

    struct Expression *newExprs = realloc(exprs->exprs, newLength * sizeof *exprs->exprs);
    if (nullptr == newExprs) {
      return false;
    }

    exprs->length = newLength;
    exprs->exprs = newExprs;
  }

  memcpy(exprs->exprs + exprs->count, expr, sizeof *expr);
  ++exprs->count;
  return true;
}


static bool lex_int(const char8_t *str, const char8_t *expectedEnd, uint64_t *value) {
  char *end = nullptr;
  *value = strtoull((const char *)str, &end, 10);
  if (ERANGE == errno || (0 == *value && str[0] != '0')) {
    return false;
  }

  str = (const char8_t *)end;
  return str == expectedEnd;
}

static bool lex(const char8_t *str, struct TokenInfo *tokens) {
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

        *   { return false; }
        end {
          // fprintf(stderr, "\n");
          return true;
        }

        // White space
        wsp = [ \t\v\n\r]+;
        wsp {
          // fprintf(stderr, "White space: '%.*s'\n", (int)(str - previous), previous);
          continue;
        }

        // Integer literal
        int = [0-9]*;
        int {
          if (!lex_int(previous, str, &intValue)) return false;
          struct Token token = {
            IntegerLiteralToken,
            .integer = intValue
          };
          add_token(tokens, &token);
          // fprintf(stderr, "Integer literal: '" PRIu64 "'\n", token.integer);
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
          // fprintf(stderr, "String literal: '%.*s'\n", token.string.strLen, token.string.str);
          continue;
        }

        // Types
        "Action" {
          struct Token token = { ActionTypeToken };
          add_token(tokens, &token);
          // fprintf(stderr, "Action type: 'Action'\n");
          continue;
        }
        "Room"   {
          struct Token token = { RoomTypeToken };
          add_token(tokens, &token);
          // fprintf(stderr, "Room type: 'Room'\n");
          continue;
        }
        "Screen" {
          struct Token token = { ScreenTypeToken };
          add_token(tokens, &token);
          // fprintf(stderr, "Screen type: 'Screen'\n");
          continue;
        }

        // Symbols
        "(" {
          struct Token token = { OpenParenToken };
          add_token(tokens, &token);
          //fprintf(stderr, "Open paren: '('\n");
          continue;
        }
        ")" {
          struct Token token = { CloseParenToken };
          add_token(tokens, &token);
          //fprintf(stderr, "Close paren: ')'\n");
          continue;
        }
        ";" {
          struct Token token = { SemicolonToken };
          add_token(tokens, &token);
          //fprintf(stderr, "Semicolon: ';'\n");
          continue;
        }
        "=" {
          struct Token token = { EqualsToken };
          add_token(tokens, &token);
          //fprintf(stderr, "Equals: '='\n");
          continue;
        }
        "," {
          struct Token token = { CommaToken };
          add_token(tokens, &token);
          //fprintf(stderr, "Comma: ','\n");
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
          // fprintf(stderr, "Identifier: '%.*s'\n", token.string.strLen, token.string.str);
          continue;
        }

        // [^=(),"\x00]+ { return TextToken; }
    */
  }
}


static bool parse(const struct TokenInfo *tokens, struct ExpressionInfo *exprs) {
  const struct Token *token = tokens->tokens;
  const struct Token *end = token + tokens->count;
  for (; token < end; ++token) {
    switch (token->type) {
      case ActionTypeToken: goto action;
      case RoomTypeToken: goto room;
      case ScreenTypeToken: goto screen;
      default: return false;
    }

action:
    ++token;
    if (token > end) {
      return false;
    }
    const struct Token *identifier = token;
    switch (token->type) {
      case IdentifierToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case EqualsToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case ActionTypeToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case OpenParenToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    const struct Token *string = token;
    switch (token->type) {
      case StringLiteralToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case CloseParenToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
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
      default: return false;
    }

room:
    ++token;
    if (token > end) {
      return false;
    }
    identifier = token;
    switch (token->type) {
      case IdentifierToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case EqualsToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case RoomTypeToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case OpenParenToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    const struct Token *integer1 = token;
    switch (token->type) {
      case IntegerLiteralToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case CommaToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    const struct Token *integer2 = token;
    switch (token->type) {
      case IntegerLiteralToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case CommaToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    string = token;
    switch (token->type) {
      case StringLiteralToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case CloseParenToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
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
      default: return false;
    }

screen:
    ++token;
    if (token > end) {
      return false;
    }
    identifier = token;
    switch (token->type) {
      case IdentifierToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case EqualsToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case ScreenTypeToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case OpenParenToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    string = token;
    switch (token->type) {
      case StringLiteralToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
      return false;
    }
    switch (token->type) {
      case CloseParenToken: break;
      default: return false;
    }

    ++token;
    if (token > end) {
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
      default: return false;
    }
  }

  return true;
}


struct String {
  const char8_t *str;
  size_t strLen;
};

static bool add_string(struct String **strings, size_t *count, size_t *length, const struct String *string) {
  if (*count + 1 >= *length) {
    size_t newLength = 2 * *count;
    if (0 == newLength) {
      newLength = 8;
    }

    struct String *newStrings = realloc(*strings, newLength * sizeof *strings);
    if (nullptr == newStrings) {
      return false;
    }

    *length = newLength;
    *strings = newStrings;
  }

  memcpy(*strings + *count, string, sizeof *string);
  ++*count;
  return true;
}

static bool codegen(const struct ExpressionInfo *exprs) {
  struct String *rooms = nullptr;
  size_t count = 0;
  size_t length = 0;

  FILE *fh = fopen("gen/ext1.h", "wb");
  if (nullptr == fh) {
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

  FILE *fc = fopen("gen/ext1.c", "wb");
  if (nullptr == fc) {
    return false;
  }

  fputs("\
#include <stddef.h>\n\
\n\
#include \"../backend.h\"\n\
#include \"ext1.h\"\n\n", fc);

  for (size_t i = 0; i < exprs->count; ++i) {
    const struct Expression *expr = exprs->exprs + i;
    switch (expr->type) {
      case ActionDefintionExpression:
      case ScreenDefinitionExpression:
        break;
      case RoomDefinitionExpression:
        struct String room = { expr->room.identifier->string.str,  expr->room.identifier->string.strLen };
        if (!add_string(&rooms, &count, &length, &room)) {
          fclose(fc);
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
  for (size_t i = 0; i < count; ++i) {
    const struct String *room = rooms + i;
    if (i > 0) {
      fputs(", ", fc);
    }
    fprintf(fc, "&Ext1_%.*s", room->strLen, room->str);
  }
  fputs(" };\n", fc);
  fputs("const size_t Ext1_RoomCount = ARR_COUNT(Ext1_Rooms);\n", fc);

  fclose(fc);
  return true;
}


int main() {
  bool status = true;
  const char path[] = "ext1.uta";
  int fd = open(path, O_RDONLY);
  if (-1 == fd) {
    return EXIT_FAILURE;
  }

  struct stat st;
  if (-1 == fstat(fd, &st)) {
    return EXIT_FAILURE;
  }

  void *file =  mmap(nullptr, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  status &= MAP_FAILED != file;

  struct TokenInfo tokens = {};
  status = status && lex(file, &tokens);

  /*if (status) {
    printTokens(&tokens);
    putchar('\n');
  }*/

  struct ExpressionInfo exprs = {};
  status = status && parse(&tokens, &exprs);

  /*if (status) {
    printExpressions(&exprs);
    putchar('\n');
  }*/

  status = status && codegen(&exprs);

  free(exprs.exprs);
  free(tokens.tokens);

  munmap(file, (size_t)st.st_size);
  close(fd);

  return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
