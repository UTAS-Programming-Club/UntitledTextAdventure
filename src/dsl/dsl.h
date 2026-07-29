#ifndef UTA_DSL_H
#define UTA_DSL_H

#include <stdint.h>  // for uint16_t, uint64_t
#include <stdio.h>   // for size_t, fprintf, stderr
#include <stdlib.h>  // for realloc                  // IWYU pragma: keep
#include <string.h>  // for memcpy                   // IWYU pragma: keep
#include <uchar.h>   // for char8_t

#define EMIT_PROG_ERROR(error, ...) \
  fprintf(stderr, "%s: \u001b[0;31merror\u001b[0m: " error "\n", programName __VA_OPT__(,) __VA_ARGS__)


#define DYN_ARRAY_DEF(typeName, baseTypeName, varName) struct typeName {                                    \
  baseTypeName *varName ## s;                                                                               \
  size_t count;                                                                                             \
  size_t length;                                                                                            \
};                                                                                                          \
                                                                                                            \
[[nodiscard]] bool add_ ## varName(struct typeName *const varName ## s, const baseTypeName *const varName);

#define DYN_ARRAY_IMPL(typeName, baseTypeName, varName)                                                      \
[[nodiscard]] bool add_ ## varName(struct typeName *const varName ## s, const baseTypeName *const varName) { \
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


extern const char *programName;
extern const char *inputPath;


struct String {
  const char8_t *str;
  size_t strLen;
};
DYN_ARRAY_DEF(StringInfo, struct String, string)


enum TokenType {
  IntegerLiteralToken,
  StringLiteralToken,

  OpenBraceToken,
  CloseBraceToken,
  OpenParenToken,
  CloseParenToken,
  SemicolonToken,
  EqualsToken,
  CommaToken,

  IdentifierToken
};


struct Token {
  enum TokenType type;
  uint16_t lineNum, colNum;
  const char8_t *const line;
  union {
    uint64_t integer;     // IntegerLiteralToken
    struct String string; // StringLiteralToken, IdentifierToken
  };
};
DYN_ARRAY_DEF(TokenInfo, struct Token, token)

[[nodiscard]] bool lex(const char8_t *restrict str, struct TokenInfo *const restrict tokens);


enum ExpressionType {
  TypeDeclarationExpression,

  ActionDefinitionExpression,
  RoomDefinitionExpression,
  ScreenDefinitionExpression
};

struct Expression {
  enum ExpressionType type;
  const struct Token *idName; // Also idBaseTypeName if type == TypeDeclerationExpression
  union {
    struct {
      const struct String *idChildTypeName;
    } typeDeclaration;
    struct {
      const struct Token *strTitle, *idVisiblityCheckerFunc, *idTriggerHandlerFunc;
      const struct String *idTypeName;
      bool isDerivedType;
    } action;
    struct {
      const struct Token *intX, *intY, *strBody;
    } room;
    struct {
      bool isBodyFunc;
      const struct Token *strBody; // Also idBodyFunc if isBodyFunc == true
      const struct TokenInfo actions;
    } screen;
  };
};
DYN_ARRAY_DEF(ExpressionInfo, struct Expression, expr)

[[nodiscard]] bool parse(const struct TokenInfo *const restrict tokens, struct ExpressionInfo *const restrict exprs);


[[nodiscard]] bool codegen(const struct ExpressionInfo *const restrict exprs, const char *const restrict headerPath, const char *const restrict sourcePath, const char *const restrict extensionName);

#endif // UTA_DSL_H
