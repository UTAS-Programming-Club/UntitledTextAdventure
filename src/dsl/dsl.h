#ifndef UTA_DSL_H
#define UTA_DSL_H

#include <stdint.h>  // for uint16_t, uint64_t
#include <stdio.h>   // for size_t, fprintf, stderr
#include <stdlib.h>  // for realloc                  // IWYU pragma: keep
#include <string.h>  // for memcpy                   // IWYU pragma: keep
#include <uchar.h>   // for char8_t

struct State; // Load bearing

#define EMIT_PROG_ERROR(error, ...) \
  fprintf(stderr, "%s: \x1b[0;31merror\x1b[0m: " error "\n", state->programName __VA_OPT__(,) __VA_ARGS__)


#define DYN_ARRAY_DEF(arrTypeName, baseTypeName, varName) struct arrTypeName {                              \
  baseTypeName *varName ## s;                                                                               \
  size_t count;                                                                                             \
  size_t length;                                                                                            \
};                                                                                                          \
                                                                                                            \
[[nodiscard]] bool add_ ## varName(const struct State *const restrict state,                                \
   struct arrTypeName *const varName ## s, const baseTypeName *const varName);

#define DYN_ARRAY_IMPL(arrTypeName, baseTypeName, varName)                                                   \
[[nodiscard]] bool add_ ## varName(const struct State *const restrict state,                                 \
   struct arrTypeName *const varName ## s, const baseTypeName *const varName) {                              \
if (varName ## s->count + 1 >= varName ## s->length) {                                                       \
    size_t newLen = 2 * varName ## s->count;                                                                 \
    if (0 == newLen) {                                                                                       \
      newLen = 8;                                                                                            \
    }                                                                                                        \
                                                                                                             \
    baseTypeName *newArr = realloc(varName ## s->varName ## s, newLen * sizeof *varName ## s->varName ## s); \
    if (nullptr == newArr) {                                                                                 \
      EMIT_PROG_ERROR("An unrecoverable error occurred");                                                    \
      return false;                                                                                          \
    }                                                                                                        \
                                                                                                             \
    varName ## s->length = newLen;                                                                           \
    varName ## s->varName ## s = newArr;                                                                     \
  }                                                                                                          \
                                                                                                             \
  memcpy(varName ## s->varName ## s + varName ## s->count, varName, sizeof *varName);                        \
  ++varName ## s->count;                                                                                     \
  return true;                                                                                               \
}


struct String {
  const char8_t *str;
  size_t len;
};

// DYN_ARRAY_DEF(StringInfo, struct String, string)


enum TokenType {
  // Literals
  BooleanLiteralToken,
  IntegerLiteralToken,
  StringLiteralToken,
  // Type Keywords
  BooleanTypeToken,
  IntegerTypeToken,
  StringTypeToken,
  EnumTypeToken,
  ActionTypeToken,
  RoomTypeToken,
  ScreenTypeToken,
  GameInfoTypeToken,
  // Keywords
  IfToken,
  WhileToken,
  ForToken,
  SwitchToken,
  CaseToken,
  ContinueToken,
  BreakToken,
  ReturnToken,
  IsToken,
  NotToken,
  NoneToken,
  NamespaceToken,
  OverrideToken,
  ConstToken,
  ThisToken,
  // Symbols
  OpenCurlyToken,
  CloseCurlyToken,
  OpenBracketToken,
  CloseBracketToken,
  SemiColonToken,
  CommaToken,
  DotToken,
  ColonToken,
  PlusToken,
  MinusToken,
  MultiplyToken,
  DivideToken,
  ModulusToken,
  LessThanToken,
  GreaterThanToken,
  LessThanEqualToken,
  GreaterThanEqualToken,
  EqualsToken,
  AssignToken,
  IdentifierToken
};

struct Token {
  const enum TokenType type;
  const uint16_t lineNum, colNum;
  const char8_t *const line;
  const bool boolean;         // BooleanLiteralToken, IntegerTypeToken for signed
  union {
    const uint64_t integer;     // IntegerLiteralToken
    const struct String string; // StringLiteralToken, IdentifierToken
  };
};
DYN_ARRAY_DEF(TokenInfo, struct Token, token)

// enum ExpressionType {
// };
// 
// struct Expression {
//   enum ExpressionType type;
// };
// DYN_ARRAY_DEF(ExpressionInfo, struct Expression, expr)


struct State {
  const char *const restrict programName;
  const char *restrict inputPath;
  struct TokenInfo tokens;
};

[[nodiscard]] bool lex(struct State *const restrict state, const char8_t *const restrict input);
[[nodiscard]] bool parse(struct State *const restrict state);
// [[nodiscard]] bool codegen(struct State *const restrict state, const char *const restrict headerPath, const char *const restrict sourcePath, const char *const restrict extensionName);

#endif // UTA_DSL_H
