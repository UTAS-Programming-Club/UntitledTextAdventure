#include <errno.h>    // for ERANGE, errno
#include <stdio.h>    // for size_t, fprintf, stderr
#include <stdlib.h>   // for strtoull
#include <uchar.h>    // for char8_t

#include "dsl/dsl.h"  // IWYU pragma: associated

#define EMIT_SRC_ERROR(error, ...) \
  fprintf(stderr, "%s: \u001b[0;31merror\u001b[0m: " error "\n", inputPath __VA_OPT__(,) __VA_ARGS__)

#define NEW_TOKEN(type, ...) {type, lineNum, (uint16_t)(previous - line), line, __VA_ARGS__}


DYN_ARRAY_IMPL(TokenInfo, struct Token, token)


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

[[nodiscard]] bool lex(const char8_t *restrict str, struct TokenInfo *const restrict tokens) {
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
