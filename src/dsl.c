#include <errno.h>     // for ERANGE, errno
#include <fcntl.h>     // for O_RDONLY, open
#include <inttypes.h>  // for uint16_t, uint64_t
#include <stdio.h>     // for fprintf, stderr, size_t, fputs, fclose, FILE, fopen, fputc, printf
#include <stdlib.h>    // for EXIT_FAILURE, free, realloc, EXIT_SUCCESS, strtoull
#include <string.h>    // for strchr, strlen, memcpy, strstr, strcmp
#include <sys/mman.h>  // for MAP_FAILED, MAP_PRIVATE, PROT_READ, mmap, munmap
#include <sys/stat.h>  // for stat, fstat
#include <uchar.h>     // for char8_t
#include <unistd.h>    // for close

static const char *programName;
static const char *inputPath;
#define EMIT_PROG_ERROR(error, ...) \
  fprintf(stderr, "%s: \u001b[0;31merror\u001b[0m: " error "\n", programName __VA_OPT__(,) __VA_ARGS__)
#define EMIT_SRC_ERROR(error, ...) \
  fprintf(stderr, "%s: \u001b[0;31merror\u001b[0m: " error "\n", inputPath __VA_OPT__(,) __VA_ARGS__)
#define EMIT_PARSE_ERROR(token, error, ...) {                                       \
  const char *str = (const char *)token->line;                                      \
  const char *end = strchr(str, '\n');                                              \
  const int strLen = nullptr == end ? (int)strlen(str) : (int)(end - str);          \
  fprintf(stderr, "%s:%w16u:%w16u: \u001b[0;31merror\u001b[0m: " error "\n",        \
    inputPath, token->lineNum + 1, token->colNum + 1 __VA_OPT__(,) __VA_ARGS__      \
  );                                                                                \
  fprintf(stderr, " %4w16u | %.*s\n", token->lineNum + 1, strLen, str);             \
  fprintf(stderr, "      |%*c\u001b[0;32m^\u001b[0;31m\n", token->colNum + 1, ' '); \
}


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
      EMIT_PROG_ERROR("An unrecoverable error occurred");                                                           \
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


struct String {
  const char8_t *str;
  size_t strLen;
};

DYN_ARRAY(StringInfo, struct String, string)


enum TokenType {
  IntegerLiteralToken,
  StringLiteralToken,

  ActionTypeToken,
  RoomTypeToken,
  ScreenTypeToken,

  OpenBraceToken,
  CloseBraceToken,
  OpenParenToken,
  CloseParenToken,
  SemicolonToken,
  EqualsToken,
  CommaToken,

  IdentifierToken
};

#define NEW_TOKEN(type, ...) {type, lineNum, (uint16_t)(previous - line), line, __VA_ARGS__}
struct Token {
  enum TokenType type;
  uint16_t lineNum, colNum;
  const char8_t *const line;
  union {
    uint64_t integer;     // IntegerLiteralToken
    struct String string; // StringLiteralToken, IdentifierToken
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

    case OpenBraceToken: return "OpenBraceToken";
    case CloseBraceToken: return "CloseBraceToken";
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
      bool isBodyFunc;
      const struct Token *strBody; // Also idBodyFunc
      const struct TokenInfo actions;
    } screen;
  };
};

DYN_ARRAY(ExpressionInfo, struct Expression, expr)


[[nodiscard]] static bool lex_int(const char8_t *str, const char8_t *const expectedEnd, uint64_t *const restrict value) {
  char *end = nullptr;
  *value = strtoull((const char *)str, &end, 10);
  if (ERANGE == errno || (0 == *value && str[0] != u8'0')) {
    EMIT_SRC_ERROR("An unexpected character was encountered in integer literal");
    return false;
  }

  str = (const char8_t *)end;
  return str == expectedEnd;
}

[[nodiscard]] static bool lex(const char8_t *restrict str, struct TokenInfo *const restrict tokens) {
  uint64_t intValue;
  const char8_t *marker = str;

  const char8_t *line = str;
  uint16_t lineNum = 0;

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
          EMIT_SRC_ERROR("An unxpected character was encountered: %c", *previous);
          return false;
        }
        end {
          return true;
        }

        // White space
        wsp = [ \t\v\r]+;
        wsp  {
          continue;
        }
        "\n" {
          line = str;
          ++lineNum;
          continue;
        }

        // Integer literal
        int = [0-9]*;
        int {
          if (!lex_int(previous, str, &intValue)) {
            return false;
          }
          struct Token token = NEW_TOKEN(
            IntegerLiteralToken,
            .integer = intValue
          );
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }

        // String literal
        str = "\"" [^\x00"]* "\"";
        str {
          struct Token token = NEW_TOKEN(
            StringLiteralToken,
            .string = {
              previous,
              (size_t)(str - previous)
            }
          );
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }

        // Types
        "Action" {
          struct Token token = NEW_TOKEN(ActionTypeToken);
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        "Room"   {
          struct Token token = NEW_TOKEN(RoomTypeToken);
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        "Screen" {
          struct Token token = NEW_TOKEN(ScreenTypeToken);
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }

        // Symbols
        "{" {
          struct Token token = NEW_TOKEN(OpenBraceToken);
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        "}" {
          struct Token token = NEW_TOKEN(CloseBraceToken);
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        "(" {
          struct Token token = NEW_TOKEN(OpenParenToken);
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        ")" {
          struct Token token = NEW_TOKEN(CloseParenToken);
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        ";" {
          struct Token token = NEW_TOKEN(SemicolonToken);
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        "=" {
          struct Token token = NEW_TOKEN(EqualsToken);
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
        "," {
          struct Token token = NEW_TOKEN(CommaToken);
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }

        // Identifier
        id = [^\x00 \t\v\n\r"{}()=,]+;
        id {
          struct Token token = NEW_TOKEN(
            IdentifierToken,
            .string = { previous, (size_t)(str - previous) }
          );
          if (!add_token(tokens, &token)) {
            return false;
          }
          continue;
        }
    */
  }
}


#define ADDITIONAL_TOKENS_ERROR() EMIT_PARSE_ERROR(token, "Additional token(s) were expected")
#define UNEXPECTED_TOKEN_ERROR()  EMIT_PARSE_ERROR(token, "An unxpected token %s was encountered", get_token_string(token->type))
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

[[nodiscard]] static bool parse(const struct TokenInfo *const restrict tokens, struct ExpressionInfo *const restrict exprs) {
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

  // Screen idName = Screen(strBody, array<Action>);
  // Screen idName = Screen(idBodyFunc, array<Action>);
screen:
    idName = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(EqualsToken);
    SINGLE_PARSE_ALLOW(ScreenTypeToken);
    SINGLE_PARSE_ALLOW(OpenParenToken);

    ++token;
    if (token > end) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }

    bool isBodyFunc = true;
    switch (token->type) {
      case StringLiteralToken:
        isBodyFunc = false;
        [[fallthrough]];
      case IdentifierToken:
        strBody = token;
        break;
      default:
        UNEXPECTED_TOKEN_ERROR();
        return false;
    }

    SINGLE_PARSE_ALLOW(CommaToken);
    SINGLE_PARSE_ALLOW(OpenBraceToken);

    struct TokenInfo actions = {};
    while (true) {
      const struct Token *const action = SINGLE_PARSE_ALLOW(IdentifierToken);
      if (!add_token(&actions, action)) {
        return false;
      }

      if (token >= end) {
        ADDITIONAL_TOKENS_ERROR();
        return false;
      }

      if (CloseBraceToken == token[1].type) {
        break;
      } else if (CommaToken == token[1].type) {
        ++token;
        continue;
      }

      UNEXPECTED_TOKEN_ERROR();
      return false;
    }

    SINGLE_PARSE_ALLOW(CloseBraceToken);
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
          .screen = { isBodyFunc, strBody, actions }
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


static void write_str(FILE *const restrict f, const size_t strLen, const char8_t str[const restrict static strLen]) {
  for (size_t i = 0; i < strLen; ++i) {
    const char8_t chr = str[i];
    if (u8'\n' == chr) {
      fputs("\\", f);
    }

    fputc(chr, f);
  }
}

[[nodiscard]] static bool codegen(const struct ExpressionInfo *const restrict exprs, const char *const restrict headerPath, const char *const restrict sourcePath, const char *const restrict extensionName) {
  struct StringInfo rooms = {};

  FILE *const fh = fopen(headerPath, "wb");
  if (nullptr == fh) {
    EMIT_PROG_ERROR("unable to open %s", headerPath);
    return false;
  }

  fprintf(fh, "\
#ifndef UTA_GEN_%s_H\n\
#define UTA_GEN_%s_H\n\
\n\
#include <stddef.h>\n\
\n", extensionName, extensionName);

  FILE *const fc = fopen(sourcePath, "wb");
  if (nullptr == fc) {
    EMIT_PROG_ERROR("unable to open %s", sourcePath);
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
        fprintf(fc, "const struct Action %s_%.*s = NEW_ACTION(",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str
        );
        write_str(fc, expr->action.strTitle->string.strLen, expr->action.strTitle->string.str);
        fprintf(fc, ", %.*s, %.*s);\n\n",
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
        fprintf(fc, "const struct Room %s_%.*s = NEW_ROOM(%w64u, %w64u, ",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str,
          expr->room.intX->integer, expr->room.intY->integer,
          (int)expr->room.strBody->string.strLen, expr->room.strBody->string.str
        );
        write_str(fc, expr->room.strBody->string.strLen, expr->room.strBody->string.str);
        fputs(");\n\n", fc);
        break;
      case ScreenDefinitionExpression:
        fprintf(fh, "extern const struct Screen %s_%.*s;\n\n",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str
        );
        fprintf(fc, "const struct Screen %s_%.*s = NEW_",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str,
          (int)expr->screen.strBody->string.strLen, expr->screen.strBody->string.str
        );
        if (expr->screen.isBodyFunc) {
          fputs("VAR_", fc);
        }
        fputs("SCREEN(", fc);
        write_str(fc, expr->screen.strBody->string.strLen, expr->screen.strBody->string.str);
        for (size_t i = 0; i < expr->screen.actions.count; ++i) {
          const struct Token *const token = expr->screen.actions.tokens + i;
          fprintf(fc, ", USE_ACTION(%.*s)", (int)token->string.strLen, token->string.str);
        }
        fputs(");\n\n", fc);
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
    return EXIT_SUCCESS;
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
    if (ScreenDefinitionExpression != expr->type) {
      continue;
    }

    free(expr->screen.actions.tokens);
  }

  free(exprs.exprs);
  free(tokens.tokens);

  munmap(file, (size_t)st.st_size);
  close(fd);

  return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
