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


#define DYN_ARRAY(typeName, baseTypeName, varName) struct typeName {                      \
  baseTypeName *varName ## s;                                                             \
  size_t count;                                                                           \
  size_t length;                                                                          \
};                                                                                        \
                                                                                          \
static bool add_ ## varName(struct typeName *varName ## s, const baseTypeName *varName) { \
if (varName ## s->count + 1 >= varName ## s->length) {                                    \
    size_t newLen = 2 * varName ## s->count;                                              \
    if (0 == newLen) {                                                                    \
      newLen = 8;                                                                         \
    }                                                                                     \
                                                                                          \
    baseTypeName *newArr = realloc(varName ## s->varName ## s, newLen * sizeof *varName); \
    if (nullptr == newArr) {                                                              \
      return false;                                                                       \
    }                                                                                     \
                                                                                          \
    varName ## s->length = newLen;                                                        \
    varName ## s->varName ## s = newArr;                                                  \
  }                                                                                       \
                                                                                          \
  memcpy(varName ## s->varName ## s + varName ## s->count, varName, sizeof *varName);     \
  ++varName ## s->count;                                                                  \
  return true;                                                                            \
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


#define SINGLE_PARSE_ALLOW(tokenType) \
  ++token;                            \
  if (token > end) {                  \
    return false;                     \
  }                                   \
                                      \
  switch (token->type) {              \
    case tokenType: break;            \
    default: return false;            \
  }                                   \
                                      \
  token

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
    const struct Token *identifier = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(EqualsToken);
    SINGLE_PARSE_ALLOW(ActionTypeToken);
    SINGLE_PARSE_ALLOW(OpenParenToken);
    const struct Token *string = SINGLE_PARSE_ALLOW(StringLiteralToken);
    SINGLE_PARSE_ALLOW(CloseParenToken);

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
    identifier = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(EqualsToken);
    SINGLE_PARSE_ALLOW(ScreenTypeToken);
    SINGLE_PARSE_ALLOW(OpenParenToken);
    string = SINGLE_PARSE_ALLOW(StringLiteralToken);
    SINGLE_PARSE_ALLOW(CloseParenToken);

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

DYN_ARRAY(StringInfo, struct String, string)

static bool codegen(const struct ExpressionInfo *exprs) {
  struct StringInfo rooms = {};

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
        if (!add_string(&rooms, &room)) {
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
  for (size_t i = 0; i < rooms.count; ++i) {
    const struct String *room = rooms.strings + i;
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

  struct ExpressionInfo exprs = {};
  status = status && parse(&tokens, &exprs);

  status = status && codegen(&exprs);

  free(exprs.exprs);
  free(tokens.tokens);

  munmap(file, (size_t)st.st_size);
  close(fd);

  return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
