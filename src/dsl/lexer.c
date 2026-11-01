#include <errno.h>    // for ERANGE, errno
#include <inttypes.h>
#include <stdio.h>    // for size_t, fprintf, stderr
#include <stdlib.h>   // for strtoull
#include <uchar.h>    // for char8_t

#include "dsl/dsl.h"  // IWYU pragma: associated

#define EMIT_SRC_ERROR(error, ...) \
  fprintf(stderr, "%s: \x1b[0;31merror\x1b[0m: " error "\n", state->inputPath __VA_OPT__(,) __VA_ARGS__)

#define NEW_TOKEN(type, ...)                                                         \
  struct Token token = {type, lineNum, (uint16_t)(start - line), line, __VA_ARGS__}; \
  if (!add_token(state, &state->tokens, &token)) {                                   \
    return false;                                                                    \
  }                                                                                  \
  continue


DYN_ARRAY_IMPL(TokenArray, struct Token, token)


[[nodiscard]] static bool lex_int(struct State *const restrict state, const char8_t *str, const char8_t *const expectedEnd, uint64_t *const restrict value) {
  char *end = nullptr;
  *value = strtoull((const char *)str, &end, 10);
  if (ERANGE == errno || (0 == *value && str[0] != u8'0')) {
    EMIT_SRC_ERROR("An unexpected character was encountered in integer literal");
    return false;
  }

  str = (const char8_t *)end;
  return str == expectedEnd;
}

// Based on the re2e c++98 demo https://re2c.org/playground/?example=c/real_world/cxx98.re
[[nodiscard]] bool lex(struct State *const restrict state, const char8_t *restrict input) {
  uint64_t intValue;
  const char8_t *marker = input;

  const char8_t *line = input;
  uint16_t lineNum = 0;

  while (true) {
    const char8_t *start = input;
    /*!re2c
        re2c:yyfill:enable = 0;
        re2c:sentinel = 0;
        re2c:define:YYCTYPE = char8_t;
        re2c:define:YYCURSOR = input;
        re2c:define:YYMARKER = marker;

        end = "\x00";

        *   {
          EMIT_SRC_ERROR("An unxpected character was encountered: %c", *start);
          return false;
        }
        end {
          return true;
        }

        // White space and comments
        // TODO: Support recursive multi line comments?
        mcm = "/*" ([^\x00*] | ("*" [^\x00/]))* "*""/";
        scm = "//" [^\x00\n]*;
        wsp = ([ \t\v\r] | scm)+;
        mcm+  {
          for (const char8_t *chr = start; chr < input; ++chr) {
            if (u8'\n' == *chr) {
              line = chr + 1;
              ++lineNum;
            }
          }
          continue;
        }
        wsp  {
          continue;
        }
        "\n" {
          line = input;
          ++lineNum;
          continue;
        }

        // Boolean literals
        "true" {
          NEW_TOKEN(
            BooleanLiteralToken,
            .boolean = true
          );
        }
        "false" {
          NEW_TOKEN(
            BooleanLiteralToken,
            .boolean = false
          );
        }

        // Integer literal
        int = [0-9]*;
        int {
          if (!lex_int(state, start, input, &intValue)) {
            return false;
          }
          NEW_TOKEN(
            IntegerLiteralToken,
            .integer = intValue
          );
        }

        // String literal
        str = "\"" [^\x00"]* "\"";
        str {
          struct Token token = {
            StringLiteralToken, 
            lineNum, (uint16_t)(start - line), line, 
            .string = { 
              start, (size_t)(input - start)
            }
          };

          if (!add_token(state, &state->tokens, &token)) {
            return false;
          }

          for (const char8_t *chr = start; chr < input; ++chr) {
            if (u8'\n' == *chr) {
              line = chr + 1;
              ++lineNum;
            }
          }

          continue;
        }

        // Type Keywords
        "bool" {
          NEW_TOKEN(BooleanTypeToken);
        }
        "int" {
          NEW_TOKEN(
            IntegerTypeToken,
            .boolean = true
          );
        }
        "uint" {
          NEW_TOKEN(
            IntegerTypeToken,
            .boolean = false
          );
        }
        "string" {
          NEW_TOKEN(StringTypeToken);
        }
        "enum" {
          NEW_TOKEN(EnumTypeToken);
        }
        "Action" {
          NEW_TOKEN(ActionTypeToken);
        }
        "Room" {
          NEW_TOKEN(RoomTypeToken);
        }
        "Screen" {
          NEW_TOKEN(ScreenTypeToken);
        }
        "GameInfo" {
          NEW_TOKEN(GameInfoTypeToken);
        }

        // Keywords
        "if" {
          NEW_TOKEN(IfToken);
        }
        "while" {
          NEW_TOKEN(WhileToken);
        }
        "for" {
          NEW_TOKEN(ForToken);
        }
        "switch" {
          NEW_TOKEN(SwitchToken);
        }
        "case" {
          NEW_TOKEN(CaseToken);
        }
        "continue" {
          NEW_TOKEN(ContinueToken);
        }
        "break" {
          NEW_TOKEN(BreakToken);
        }
        "return" {
          NEW_TOKEN(ReturnToken);
        }
        "is" {
          NEW_TOKEN(IsToken);
        }
        "not" {
          NEW_TOKEN(NotToken);
        }
        "none" {
          NEW_TOKEN(NoneToken);
        }
        "namespace" {
          NEW_TOKEN(NamespaceToken);
        }
        "override" {
          NEW_TOKEN(OverrideToken);
        }
        "const" {
          NEW_TOKEN(ConstToken);
        }
        "this" {
          NEW_TOKEN(ThisToken);
        }

        // Symbols
        "{" {
          NEW_TOKEN(OpenCurlyToken);
        }
        "}" {
          NEW_TOKEN(CloseCurlyToken);
        }
        "(" {
          NEW_TOKEN(OpenBracketToken);
        }
        ")" {
          NEW_TOKEN(CloseBracketToken);
        }
        ";" {
          NEW_TOKEN(SemiColonToken);
        }
        "," {
          NEW_TOKEN(CommaToken);
        }
        "." {
          NEW_TOKEN(DotToken);
        }
        ":" {
          NEW_TOKEN(ColonToken);
        }
        "+" {
          NEW_TOKEN(PlusToken);
        }
        "-" {
          NEW_TOKEN(MinusToken);
        }
        "*" {
          NEW_TOKEN(MultiplyToken);
        }
        "/" {
          NEW_TOKEN(DivideToken);
        }
        "%" {
          NEW_TOKEN(ModulusToken);
        }
        "<" {
          NEW_TOKEN(LessThanToken);
        }
        ">" {
          NEW_TOKEN(GreaterThanToken);
        }
        "<=" {
          NEW_TOKEN(LessThanEqualToken);
        }
        ">=" {
          NEW_TOKEN(GreaterThanEqualToken);
        }
        "==" {
          NEW_TOKEN(EqualsToken);
        }
        "=" {
          NEW_TOKEN(AssignToken);
        }

        // Identifier
        id = [^\x00 \t\v\n\r"{}();:=,.]+;
        id {
          NEW_TOKEN(
            IdentifierToken,
            .string = { start, (size_t)(input - start) }
          );
        }
    */
  }
}
