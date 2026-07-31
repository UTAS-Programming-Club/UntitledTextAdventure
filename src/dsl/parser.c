#include <inttypes.h>  // for PRIu16
#include <stdio.h>     // for fprintf, stderr, size_t
#include <string.h>    // for strchr, strlen, strncmp

#include "dsl/dsl.h"   // IWYU pragma: associated

#define EMIT_PARSE_ERROR(token, error, ...) {                                            \
  const char *const str = (typeof(str))token->line;                                      \
  const char *end = strchr(str, '\n');                                                   \
  const int strLen = nullptr == end ? (int)strlen(str) : (int)(end - str);               \
  fprintf(stderr, "%s:%" PRIu16 ":%" PRIu16 ": \u001b[0;31merror:\u001b[0m " error "\n", \
    inputPath, token->lineNum + 1, token->colNum + 1 __VA_OPT__(,) __VA_ARGS__           \
  );                                                                                     \
  fprintf(stderr, " %4" PRIu16 " | %.*s\n", token->lineNum + 1, strLen, str);            \
  fprintf(stderr, "      |%*c\u001b[0;32m^\u001b[0m\n", token->colNum + 1, ' ');         \
}

#define ADDITIONAL_TOKENS_ERROR() EMIT_PARSE_ERROR((tokens->tokens + *idx), "Additional token(s) were expected")
#define UNEXPECTED_TOKEN_ERROR()  EMIT_PARSE_ERROR((tokens->tokens + *idx), "An unxpected token %s was encountered", get_token_string((tokens->tokens + *idx)->type))

#define DEFAULT_PARSE_ERROR()   \
    default:                    \
      UNEXPECTED_TOKEN_ERROR(); \
      return false

#define PARSE_BLOCK                                     \
  tokens->tokens + *idx + 1;                            \
  if (*idx + 1 >= tokens->count) {                      \
    ADDITIONAL_TOKENS_ERROR();                          \
    return false;                                       \
  }                                                     \
  ++(*idx);                                             \
                                                        \
  switch (tokens->tokens[*idx].type)

#define SINGLE_PARSE_ALLOW(tokenType) \
  PARSE_BLOCK {                       \
    case tokenType: break;            \
    DEFAULT_PARSE_ERROR();            \
  }

#define GET_TOKEN(...) (tokens->tokens + *idx __VA_OPT__(+) __VA_ARGS__)


DYN_ARRAY_IMPL(ExpressionInfo, struct Expression, expr)


// TODO: Move these to params?
static const struct String actionTypeName = { u8"Action", 6 };
static const struct String roomTypeName = { u8"Room", 4 };
static const struct String screenTypeName = { u8"Screen", 6 };
static struct StringInfo actionTypeNames = {};
static struct StringInfo roomTypeNames = {};
static struct StringInfo screenTypeNames = {};


static const char *get_token_string(enum TokenType token) {
  switch (token) {
    case IntegerLiteralToken: return "IntegerLiteralToken";
    case StringLiteralToken:  return "StringLiteralToken";

    case EnumKeywordToken: return "EnumKeywordToken";

    case OpenBraceToken:  return "OpenBraceToken";
    case CloseBraceToken: return "CloseBraceToken";
    case OpenParenToken:  return "OpenParenToken";
    case CloseParenToken: return "CloseParenToken";
    case SemicolonToken:  return "SemicolonToken";
    case EqualsToken:     return "EqualsToken";
    case CommaToken:      return "CommaToken";

    case IdentifierToken: return "IdentifierToken";
  }
}


static bool string_equals(const struct String *const restrict str1, const struct String * const restrict str2) {
  if (str1->strLen != str2->strLen) {
    return false;
  }

  return 0 == strncmp((const char *)str1->str, (const char *)str2->str, str1->strLen);
}

static bool is_type_name_known(const struct StringInfo *const restrict typeNames, const struct Token *const restrict idTypeName) {
  for (size_t i = 0; i < typeNames->count; ++i) {
    if (string_equals(typeNames->strings + i, &idTypeName->string)) {
      return true;
    }
  }

  return false;
}


/* enum idEnumName {
 *   value;
 *   ...
 * }
 */
[[nodiscard]] static bool parse_enumdef(const struct TokenInfo *const restrict tokens, size_t *const restrict idx, struct ExpressionInfo *const restrict exprs) {
  const struct Token *const idEnumName = SINGLE_PARSE_ALLOW(IdentifierToken);
  SINGLE_PARSE_ALLOW(OpenBraceToken);

  struct TokenInfo idValues = {};
  const struct Token *const idValue = SINGLE_PARSE_ALLOW(IdentifierToken);
  if (!add_token(&idValues, idValue)) {
    return false;
  }

  while (true) {
    SINGLE_PARSE_ALLOW(CommaToken);

    const struct Token *const idValue = SINGLE_PARSE_ALLOW(IdentifierToken);
    if (!add_token(&idValues, idValue)) {
      return false;
    }

    if (*idx >= tokens->count) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }

    if (CloseBraceToken == GET_TOKEN(1)->type) {
      break;
    }
  }

  PARSE_BLOCK {
    case CloseBraceToken:
      const struct Expression expr = {
        EnumDefinitionExpression, idEnumName,
        .idValues = idValues
      };
      return add_expr(exprs, &expr);
    DEFAULT_PARSE_ERROR();
  }
}

/* idBaseTypeName idChildTypeName {
 *   type name;
 *   ...
 *  }
 */
[[nodiscard]] static bool parse_typedef(const struct TokenInfo *const restrict tokens, size_t *const restrict idx, struct ExpressionInfo *const restrict exprs,
                                        const struct Token *const restrict idBaseTypeName, struct StringInfo *const restrict existingTypeNames,
                                        const struct Token *const restrict idChildTypeName) {
  if (is_type_name_known(&actionTypeNames, idChildTypeName) ||
      is_type_name_known(&roomTypeNames,   idChildTypeName) ||
      is_type_name_known(&screenTypeNames, idChildTypeName)) {
      UNEXPECTED_TOKEN_ERROR();
      return false;
  }

  struct String idVields = {};
  while (true) {
    if (*idx >= tokens->count) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }

    if (CloseBraceToken == GET_TOKEN(1)->type) {
      break;
    }

    const struct Token *const idVariableTypeNameStart = SINGLE_PARSE_ALLOW(IdentifierToken);
    for (; *idx < tokens->count && SemicolonToken != GET_TOKEN()->type; ++*idx) {
    }
    if (*idx >= tokens->count) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }

    const struct Token *const idEndOfField = GET_TOKEN();
    if (0 == idVields.strLen) {
      idVields.str = idVariableTypeNameStart->string.str;
    }
    idVields.strLen = (size_t)(idEndOfField->line + idEndOfField->colNum + 1 - idVields.str);
  }

  PARSE_BLOCK {
    case CloseBraceToken:
      if (!add_string(existingTypeNames, &idChildTypeName->string)) {
        return false;
      }

      const struct Expression expr = {
        TypeDefinitionExpression, idBaseTypeName,
        .typeDefinition = { &idChildTypeName->string, idVields }
      };
      return add_expr(exprs, &expr);
    DEFAULT_PARSE_ERROR();
  }
}

// idTypeName idName = idTypeName(strTitle, idVisiblityCheckerFunc, idTriggerHandlerFunc);
// Action idChildTypeName { }
[[nodiscard]] static bool parse_action(const struct TokenInfo *const restrict tokens, size_t *const restrict idx, struct ExpressionInfo *const restrict exprs,
                                       const struct Token *const restrict idTypeName) {
  const struct Token *const idName = SINGLE_PARSE_ALLOW(IdentifierToken);

  PARSE_BLOCK {
    case OpenBraceToken:
      return parse_typedef(tokens, idx, exprs, idTypeName, &actionTypeNames, idName);
    case EqualsToken:
      break;
    DEFAULT_PARSE_ERROR();
  }

  const struct Token *const idTypeName2 = SINGLE_PARSE_ALLOW(IdentifierToken);
  if (!string_equals(&idTypeName->string, &idTypeName2->string)) {
    UNEXPECTED_TOKEN_ERROR();
    return false;
  }

  SINGLE_PARSE_ALLOW(OpenParenToken);
  const struct Token *const strTitle = SINGLE_PARSE_ALLOW(StringLiteralToken);
  SINGLE_PARSE_ALLOW(CommaToken);
  const struct Token *const idVisiblityCheckerFunc = SINGLE_PARSE_ALLOW(IdentifierToken);
  SINGLE_PARSE_ALLOW(CommaToken);
  const struct Token *const idTriggerHandlerFunc = SINGLE_PARSE_ALLOW(IdentifierToken);

  struct TokenInfo idParams = {};
  while (true) {
    if (*idx >= tokens->count) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }

    if (CloseParenToken == GET_TOKEN(1)->type) {
      break;
    }

    SINGLE_PARSE_ALLOW(CommaToken);

    const struct Token *const idParam = SINGLE_PARSE_ALLOW(IdentifierToken);
    if (!add_token(&idParams, idParam)) {
      return false;
    }
  }

  SINGLE_PARSE_ALLOW(CloseParenToken);

  PARSE_BLOCK {
    case SemicolonToken:
      const bool isDerivedType = !string_equals(&actionTypeName, &idTypeName->string);
      const struct Expression expr = {
        ActionDefinitionExpression, idName,
        .action = {
          strTitle,
          idVisiblityCheckerFunc, idTriggerHandlerFunc,
          idTypeName, isDerivedType,
          idParams
        }
      };
      return add_expr(exprs, &expr);
    DEFAULT_PARSE_ERROR();
  }
}

// idTypeName idName = idTypeName(intX, intY, stringBody);
// Room idChildTypeName { }
[[nodiscard]] static bool parse_room(const struct TokenInfo *const restrict tokens, size_t *const restrict idx, struct ExpressionInfo *const restrict exprs,
                                      const struct Token *const restrict idTypeName) {
  const struct Token *const idName = SINGLE_PARSE_ALLOW(IdentifierToken);

  PARSE_BLOCK {
    case OpenBraceToken:
      return parse_typedef(tokens, idx, exprs, idTypeName, &roomTypeNames, idName);
    case EqualsToken:
      break;
    DEFAULT_PARSE_ERROR();
  }

  const struct Token *const idTypeName2 = SINGLE_PARSE_ALLOW(IdentifierToken);
  if (!string_equals(&idTypeName->string, &idTypeName2->string)) {
    UNEXPECTED_TOKEN_ERROR();
    return false;
  }

  SINGLE_PARSE_ALLOW(OpenParenToken);
  const struct Token *const intX = SINGLE_PARSE_ALLOW(IntegerLiteralToken);
  SINGLE_PARSE_ALLOW(CommaToken);
  const struct Token *const intY = SINGLE_PARSE_ALLOW(IntegerLiteralToken);
  SINGLE_PARSE_ALLOW(CommaToken);
  const struct Token *const strBody = SINGLE_PARSE_ALLOW(StringLiteralToken);
  SINGLE_PARSE_ALLOW(CloseParenToken);

  PARSE_BLOCK {
    case SemicolonToken:
      const bool isDerivedType = !string_equals(&roomTypeName, &idTypeName->string);
      const struct Expression expr = {
        RoomDefinitionExpression, idName,
        .room = {
          intX, intY, strBody,
          idTypeName, isDerivedType
        }
      };
      return add_expr(exprs, &expr);
    DEFAULT_PARSE_ERROR();
  }
}

// idTypeName idName = idTypeName(strBody, array<Action>);
// idTypeName idName = idTypeName(idBodyFunc, array<Action>);
// Screen idChildTypeName { }
[[nodiscard]] static bool parse_screen(const struct TokenInfo *const restrict tokens, size_t *const restrict idx, struct ExpressionInfo *const restrict exprs,
                                      const struct Token *const restrict idTypeName) {
  const struct Token *const idName = SINGLE_PARSE_ALLOW(IdentifierToken);

  PARSE_BLOCK {
    case OpenBraceToken:
      return parse_typedef(tokens, idx, exprs, idTypeName, &screenTypeNames, idName);
    case EqualsToken:
      break;
    DEFAULT_PARSE_ERROR();
  }

  const struct Token *const idTypeName2 = SINGLE_PARSE_ALLOW(IdentifierToken);
  if (!string_equals(&idTypeName->string, &idTypeName2->string)) {
    UNEXPECTED_TOKEN_ERROR();
    return false;
  }

  SINGLE_PARSE_ALLOW(OpenParenToken);

  bool isBodyFunc = true;
  PARSE_BLOCK {
    case StringLiteralToken:
      isBodyFunc = false;
      break;
    case IdentifierToken:
      break;
    DEFAULT_PARSE_ERROR();
  }
  const struct Token *const strBody = GET_TOKEN();

  SINGLE_PARSE_ALLOW(CommaToken);
  SINGLE_PARSE_ALLOW(OpenBraceToken);

  struct TokenInfo actions = {};
  while (true) {
    const struct Token *const action = SINGLE_PARSE_ALLOW(IdentifierToken);
    if (!add_token(&actions, action)) {
      return false;
    }

    if (*idx >= tokens->count) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }

    if (CloseBraceToken == GET_TOKEN(1)->type) {
      break;
    } else if (CommaToken == GET_TOKEN(1)->type) {
      ++(*idx);
      continue;
    }

    UNEXPECTED_TOKEN_ERROR();
    return false;
  }

  SINGLE_PARSE_ALLOW(CloseBraceToken);
  SINGLE_PARSE_ALLOW(CloseParenToken);

  PARSE_BLOCK {
    case SemicolonToken:
      const bool isDerivedType = !string_equals(&screenTypeName, &idTypeName->string);
      const struct Expression expr = {
        ScreenDefinitionExpression, idName,
        .screen = {
          isBodyFunc, strBody,
          idTypeName, isDerivedType,
          actions
        }
      };
      return add_expr(exprs, &expr);
    DEFAULT_PARSE_ERROR();
  }
}

[[nodiscard]] bool parse(const struct TokenInfo *const restrict tokens, struct ExpressionInfo *const restrict exprs) {
  if (!add_string(&actionTypeNames, &actionTypeName) ||
      !add_string(&roomTypeNames,   &roomTypeName)   ||
      !add_string(&screenTypeNames, &screenTypeName)) {
    return false;
  }

  size_t i;
  size_t *idx = &i;
  for (i = 0; i < tokens->count; ++i) {
    const struct Token *const token = tokens->tokens + i;
    switch (token->type) {
      case EnumKeywordToken:
        if (!parse_enumdef(tokens, idx, exprs)) {
          return false;
        }
        break;
      case IdentifierToken:
        if (is_type_name_known(&actionTypeNames, token)) {
          if (!parse_action(tokens, idx, exprs, token)) {
            return false;
          }
        } else if (is_type_name_known(&roomTypeNames, token)) {
          if (!parse_room(tokens, idx, exprs, token)) {
            return false;
          }
        } else if (is_type_name_known(&screenTypeNames, token)) {
          if (!parse_screen(tokens, idx, exprs, token)) {
            return false;
          }
        } else {
          UNEXPECTED_TOKEN_ERROR();
          return false;
        }

        break;
      DEFAULT_PARSE_ERROR();
    }
  }

  free(actionTypeNames.strings);
  free(roomTypeNames.strings);
  free(screenTypeNames.strings);
  // Uncomment if calling parse more than once for some reason, or perhaps move freeing to end of program
  // actionTypeNames.strings = roomTypeNames.strings = screenTypeNames.strings = nullptr;
  // actionTypeNames.count = roomTypeNames.count = screenTypeNames.count = 0;
  // actionTypeNames.length = roomTypeNames.length = screenTypeNames.length = 0;

  return true;
}
