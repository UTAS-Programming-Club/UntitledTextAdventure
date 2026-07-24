#include <errno.h>     // for ERANGE, errno
#include <fcntl.h>     // for O_RDONLY, open
#include <inttypes.h>  // for uint64_t, PRIu64
#include <stdio.h>     // for fprintf, stderr, size_t, fclose, FILE, fopen, fputs, printf
#include <stdlib.h>    // for EXIT_FAILURE, free, realloc, EXIT_SUCCESS, strtoull
#include <string.h>    // for memcpy, strstr, strcmp
#include <sys/mman.h>  // for MAP_FAILED, MAP_PRIVATE, PROT_READ, mmap, munmap
#include <sys/stat.h>  // for stat, fstat
#include <uchar.h>     // for char8_t
#include <unistd.h>    // for close

static const char *programName;
#define EMIT_ERROR(error, ...) \
  fprintf(stderr, "%s: \u001b[0;31merror\u001b[0m: " error "\n", programName __VA_OPT__(,) __VA_ARGS__)

#define DYN_ARRAY(typeName, baseTypeName, varName) struct typeName {                                                \
  baseTypeName *varName ## s;                                                                                       \
  size_t count;                                                                                                     \
  size_t length;                                                                                                    \
};                                                                                                                  \
                                                                                                                    \
[[nodiscard]] static bool add_ ## varName(struct typeName *const varName ## s, const baseTypeName *const varName) { \
if (varName ## s->count + 1 >= varName ## s->length) {                                                              \
    size_t newLen = 2 * varName ## s->count;                                                                        \
    if (0 == newLen) {                                                                                              \
      newLen = 8;                                                                                                   \
    }                                                                                                               \
                                                                                                                    \
    baseTypeName *newArr = realloc(varName ## s->varName ## s, newLen * sizeof *varName ## s->varName ## s);        \
    if (nullptr == newArr) {                                                                                        \
      EMIT_ERROR("An unrecoverable error occurred");                                                                \
      return false;                                                                                                 \
    }                                                                                                               \
                                                                                                                    \
    varName ## s->length = newLen;                                                                                  \
    varName ## s->varName ## s = newArr;                                                                            \
  }                                                                                                                 \
                                                                                                                    \
  memcpy(varName ## s->varName ## s + varName ## s->count, varName, sizeof *varName);                               \
  ++varName ## s->count;                                                                                            \
  return true;                                                                                                      \
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

static const char *get_token_string(enum TokenType token) {
  switch (token) {
    case IntegerLiteralToken: return "IntegerLiteralToken";
    case StringLiteralToken: return "StringLiteralToken";

    case ActionTypeToken: return "ActionTypeToken";
    case RoomTypeToken: return "RoomTypeToken";
    case ScreenTypeToken: return "ScreenTypeToken";

    case OpenParenToken: return "OpenParenToken";
    case CloseParenToken: return "CloseParenToken";
    case SemicolonToken: return "SemicolonToken";
    case EqualsToken: return "EqualsToken";
    case CommaToken: return "CommaToken";

    case IdentifierToken: return "IdentifierToken";
  }
}


enum ExpressionType {
  ActionDefintionExpression,
  RoomDefinitionExpression,
  ScreenDefinitionExpression
};

struct Expression {
  enum ExpressionType type;
  const struct Token *idName;
  union {
    struct {
      const struct Token *strTitle, *idVisiblityCheckerFunc, *idTriggerHandlerFunc;
    } action;
    struct {
      const struct Token *intX, *intY, *strBody;
    } room;
    struct {
      const struct Token *strBody;
    } screen;
  };
};

DYN_ARRAY(ExpressionInfo, struct Expression, expr)


[[nodiscard]] static bool lex_int(const char8_t *str, const char8_t *const expectedEnd, uint64_t *const value) {
  char *end = nullptr;
  *value = strtoull((const char *)str, &end, 10);
  if (ERANGE == errno || (0 == *value && str[0] != '0')) {
    EMIT_ERROR("An unexpected character was encountered in integer literal");
    return false;
  }

  str = (const char8_t *)end;
  return str == expectedEnd;
}

[[nodiscard]] static bool lex(const char8_t *str, struct TokenInfo *const tokens) {
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
          EMIT_ERROR("An unxpected character was encountered");
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
          if (!add_token(tokens, &token)) {
            return false;
          }
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
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }

        // Types
        "Action" {
          struct Token token = { ActionTypeToken };
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        "Room"   {
          struct Token token = { RoomTypeToken };
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        "Screen" {
          struct Token token = { ScreenTypeToken };
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }

        // Symbols
        "(" {
          struct Token token = { OpenParenToken };
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        ")" {
          struct Token token = { CloseParenToken };
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        ";" {
          struct Token token = { SemicolonToken };
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        "=" {
          struct Token token = { EqualsToken };
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        "," {
          struct Token token = { CommaToken };
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }

        // Identifier
        id = [^\x00 \t\v\n\r"()=,]+;
        id {
          struct Token token = {
            IdentifierToken,
            .string = { previous, (size_t)(str - previous) }
          };
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }

        // [^=(),"\x00]+ { return TextToken; }
    */
  }
}


#define ADDITIONAL_TOKENS_ERROR() EMIT_ERROR("Additional token(s) were expected")
#define UNEXPECTED_TOKEN_ERROR()  EMIT_ERROR("An unxpected token %s was encountered", get_token_string(token->type))
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

[[nodiscard]] static bool parse(const struct TokenInfo *const tokens, struct ExpressionInfo *const exprs) {
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

  // Action idName = Action(strTitle, idVisiblityCheckerFunc, idTriggerHandlerFunc);
action:
    const struct Token *idName = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(EqualsToken);
    SINGLE_PARSE_ALLOW(ActionTypeToken);
    SINGLE_PARSE_ALLOW(OpenParenToken);
    const struct Token *strTitle = SINGLE_PARSE_ALLOW(StringLiteralToken);
    SINGLE_PARSE_ALLOW(CommaToken);
    const struct Token *idVisiblityCheckerFunc = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(CommaToken);
    const struct Token *idTriggerHandlerFunc = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(CloseParenToken);

    ++token;
    if (token > end) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }
    switch (token->type) {
      case SemicolonToken:
        struct Expression expr = {
          ActionDefintionExpression, idName,
          .action = { strTitle, idVisiblityCheckerFunc, idTriggerHandlerFunc }
        };
        if (!add_expr(exprs, &expr)) {
          return false;
        }
        continue;
      default:
        UNEXPECTED_TOKEN_ERROR();
        return false;
    }

  // Room idName = Room(intX, intY, stringBody);
room:
    idName = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(EqualsToken);
    SINGLE_PARSE_ALLOW(RoomTypeToken);
    SINGLE_PARSE_ALLOW(OpenParenToken);
    const struct Token *intX = SINGLE_PARSE_ALLOW(IntegerLiteralToken);
    SINGLE_PARSE_ALLOW(CommaToken);
    const struct Token *intY = SINGLE_PARSE_ALLOW(IntegerLiteralToken);
    SINGLE_PARSE_ALLOW(CommaToken);
    const struct Token *strBody = SINGLE_PARSE_ALLOW(StringLiteralToken);
    SINGLE_PARSE_ALLOW(CloseParenToken);

    ++token;
    if (token > end) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }
    switch (token->type) {
      case SemicolonToken:
        struct Expression expr = {
          RoomDefinitionExpression, idName,
          .room = { intX, intY, strBody }
        };
        if (!add_expr(exprs, &expr)) {
          return false;
        }
        continue;
      default:
        UNEXPECTED_TOKEN_ERROR();
        return false;
    }

  // Screen idName = Screen(strBody);
screen:
    idName = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(EqualsToken);
    SINGLE_PARSE_ALLOW(ScreenTypeToken);
    SINGLE_PARSE_ALLOW(OpenParenToken);
    strBody = SINGLE_PARSE_ALLOW(StringLiteralToken);
    SINGLE_PARSE_ALLOW(CloseParenToken);

    ++token;
    if (token > end) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }
    switch (token->type) {
      case SemicolonToken:
        struct Expression expr = {
          ScreenDefinitionExpression, idName,
          .screen = { strBody }
        };
        if (!add_expr(exprs, &expr)) {
          return false;
        }
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

static bool codegen(const struct ExpressionInfo *const exprs, const char *const headerPath, const char *const sourcePath, const char *const extensionName) {
  struct StringInfo rooms = {};

  FILE *const fh = fopen(headerPath, "wb");
  if (nullptr == fh) {
    EMIT_ERROR("unable to open %s", headerPath);
    return false;
  }

  fprintf(fh, "\
#ifndef UTA_GEN_%s_H\n\
#define UTA_GEN_%s_H\n\
\n\
#include <stddef.h>\n\
\n", extensionName, extensionName, extensionName);

  FILE *const fc = fopen(sourcePath, "wb");
  if (nullptr == fc) {
    EMIT_ERROR("unable to open %s", sourcePath);
    fclose(fh);
    return false;
  }

  fprintf(fc, "\
#include <stddef.h>\n\
\n\
#include \"backend.h\"\n\
#include \"coregame2.h\"\n\
#include \"%s\"\n\n", headerPath);

  for (size_t i = 0; i < exprs->count; ++i) {
    const struct Expression *const expr = exprs->exprs + i;
    switch (expr->type) {
      case ActionDefintionExpression:
        fprintf(fh, "extern const struct Action %s_%.*s;\n\n",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str
        );
        fprintf(fc, "const struct Action %s_%.*s = NEW_ACTION(%.*s, %.*s, %.*s);\n\n",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str,
          (int)expr->action.strTitle->string.strLen, expr->action.strTitle->string.str,
          (int)expr->action.idVisiblityCheckerFunc->string.strLen, expr->action.idVisiblityCheckerFunc->string.str,
          (int)expr->action.idTriggerHandlerFunc->string.strLen, expr->action.idTriggerHandlerFunc->string.str
        );
        break;
      case RoomDefinitionExpression:
        struct String room = { expr->idName->string.str, expr->idName->string.strLen };
        if (!add_string(&rooms, &room)) {
          free(rooms.strings);
          fclose(fc);
          fclose(fh);
          return false;
        }
        fprintf(fh, "extern const struct Room %s_%.*s;\n\n",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str
        );
        fprintf(fc, "const struct Room %s_%.*s = NEW_ROOM(%" PRIu64", %" PRIu64", %.*s);\n\n",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str,
          expr->room.intX->integer, expr->room.intY->integer,
          (int)expr->room.strBody->string.strLen, expr->room.strBody->string.str
        );
        break;
      case ScreenDefinitionExpression:
        EMIT_ERROR("Screen defintions are not currently supported");
        break;
    }
  }

  fprintf(fc, "const struct Room *const %s_Rooms[] = { ", extensionName);
  for (size_t i = 0; i < rooms.count; ++i) {
    const struct String *const room = rooms.strings + i;
    if (i > 0) {
      fputs(", ", fc);
    }
    fprintf(fc, "&%s_%.*s", extensionName, (int)room->strLen, room->str);
  }
  fputs(" };\n", fc);
  fprintf(fc, "const size_t %s_RoomCount = ARR_COUNT(%s_Rooms);\n", extensionName, extensionName);

  fprintf(fh, "\
extern const size_t %s_RoomCount;\n\
extern const struct Room *const %s_Rooms[];\n\
\n\
#endif // UTA_GEN_%s_H\n", extensionName, extensionName, extensionName);

  free(rooms.strings);
  fclose(fc);
  fclose(fh);
  return true;
}


#define USAGE "Usage: %s input output_header output_source extension_name\n"
#define PATH_CHECK(name, idx, expectedExt, error)                             \
  const char *const name = argv[idx];                                         \
  ext = strstr(name, expectedExt);                                            \
  if (nullptr == ext || name == ext || '\0' != ext[sizeof expectedExt - 1]) { \
    fprintf(stderr, USAGE, argv[0]);                                          \
    EMIT_ERROR(error);                                                        \
    return EXIT_FAILURE;                                                      \
  }

int main(const int argc, const char *const argv[argc]) {
  programName = argv[0];

  bool status = true;
  if (1 == argc || (2 == argc && 0 == strcmp(argv[1], "-h"))) {
    printf(USAGE, argv[0]);
    return EXIT_SUCCESS;
  }

  if (5 != argc) {
    fprintf(stderr, USAGE, argv[0]);
    return EXIT_FAILURE;
  }

  const char *ext;
  PATH_CHECK(inputPath,   1, ".uta", "input must be a uta source file");
  PATH_CHECK(outputHPath, 2,   ".h", "output header must be a c header file");
  PATH_CHECK(outputCPath, 3,   ".c", "output source must be a c source file");

  const char *const extensionName = argv[4];

  int fd = open(inputPath, O_RDONLY);
  if (-1 == fd) {
    EMIT_ERROR("unable to open %s", inputPath);
    return EXIT_FAILURE;
  }

  struct stat st;
  if (-1 == fstat(fd, &st)) {
    EMIT_ERROR("unable to process %s", inputPath);
    return EXIT_FAILURE;
  }

  void *file = mmap(nullptr, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  status &= MAP_FAILED != file;
  if (!status) {
    EMIT_ERROR("unable to read from %s", inputPath);
  }

  struct TokenInfo tokens = {};
  status = status && lex(file, &tokens);

  struct ExpressionInfo exprs = {};
  status = status && parse(&tokens, &exprs);

  status = status && codegen(&exprs, outputHPath, outputCPath, extensionName);

  free(exprs.exprs);
  free(tokens.tokens);

  munmap(file, (size_t)st.st_size);
  close(fd);

  return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
