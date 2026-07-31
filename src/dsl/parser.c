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

#define ADDITIONAL_TOKENS_ERROR() EMIT_PARSE_ERROR(GET_TOKEN(), "Additional token(s) were expected")
#define UNEXPECTED_TOKEN_ERROR()  EMIT_PARSE_ERROR(GET_TOKEN(), "An unxpected token %s was encountered", get_token_string(GET_TOKEN()->type))

#define DEFAULT_PARSE_ERROR()   \
    default:                    \
      UNEXPECTED_TOKEN_ERROR(); \
      return false


#define GET_TOKEN() (tokens->tokens + *idx)
#define PEAK_TOKEN()               \
  tokens->tokens + *idx + 1;       \
  if (*idx + 1 >= tokens->count) { \
    ADDITIONAL_TOKENS_ERROR();     \
    return false;                  \
  }

#define PARSE_BLOCK                  \
  PEAK_TOKEN();                      \
  ++(*idx);                          \
                                     \
  switch (tokens->tokens[*idx].type)

#define SINGLE_PARSE_ALLOW(tokenType) \
  PARSE_BLOCK {                       \
    case tokenType: break;            \
    DEFAULT_PARSE_ERROR();            \
  }


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

    case ActionKeywordToken: return "ActionKeywordToken";
    case EnumKeywordToken: return "EnumKeywordToken";
    case RoomKeywordToken: return "RoomKeywordToken";
    case ScreenKeywordToken: return "ScreenKeywordToken";

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
 *   value,
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

    const struct Token *const nextToken = PEAK_TOKEN();
    if (CloseBraceToken == nextToken->type) {
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
                                        const struct String *const restrict idBaseTypeName, struct StringInfo *const restrict existingTypeNames,
                                        const struct Token *const restrict idChildTypeName) {
  if (is_type_name_known(&actionTypeNames, idChildTypeName) ||
      is_type_name_known(&roomTypeNames,   idChildTypeName) ||
      is_type_name_known(&screenTypeNames, idChildTypeName)) {
      UNEXPECTED_TOKEN_ERROR();
      return false;
  }

  struct String idFields = {};
  while (true) {
    const struct Token *const nextToken = PEAK_TOKEN();
    if (CloseBraceToken == nextToken->type) {
      break;
    }

    ++(*idx);

    const struct Token *idVariableTypeNameStart = GET_TOKEN();
    for (; *idx < tokens->count && SemicolonToken != GET_TOKEN()->type; ++*idx) {
    }
    if (*idx >= tokens->count) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }

    const struct Token *const idEndOfField = GET_TOKEN();
    if (0 == idFields.strLen) {
      idFields.str = idVariableTypeNameStart->line + idVariableTypeNameStart->colNum;
    }
    idFields.strLen = (size_t)(idEndOfField->line + idEndOfField->colNum + 1 - idFields.str);
  }

  PARSE_BLOCK {
    case CloseBraceToken:
      if (!add_string(existingTypeNames, &idChildTypeName->string)) {
        return false;
      }

      const struct Expression expr = {
        TypeDefinitionExpression, idChildTypeName,
        .typeDefinition = { idBaseTypeName, idFields }
      };
      return add_expr(exprs, &expr);
    DEFAULT_PARSE_ERROR();
  }
}

// Action idName = idTypeName(strTitle, idVisiblityCheckerFunc, idTriggerHandlerFunc);
// Action idChildTypeName { }
[[nodiscard]] static bool parse_action(const struct TokenInfo *const restrict tokens, size_t *const restrict idx, struct ExpressionInfo *const restrict exprs) {
  const struct Token *const idName = SINGLE_PARSE_ALLOW(IdentifierToken);

  PARSE_BLOCK {
    case OpenBraceToken:
      return parse_typedef(tokens, idx, exprs, &actionTypeName, &actionTypeNames, idName);
    case EqualsToken:
      break;
    DEFAULT_PARSE_ERROR();
  }

  const struct String *idTypeName;
  bool isDerivedType;
  PARSE_BLOCK {
    case ActionKeywordToken:
      idTypeName = &actionTypeName;
      isDerivedType = false;
      break;
    case IdentifierToken:
      const struct Token *const idTypeNameToken = GET_TOKEN();
      if (!is_type_name_known(&actionTypeNames, idTypeNameToken)) {
        UNEXPECTED_TOKEN_ERROR();
        return false;
      }

      idTypeName = &idTypeNameToken->string;
      isDerivedType = true;
      break;
    DEFAULT_PARSE_ERROR();
  }

  SINGLE_PARSE_ALLOW(OpenParenToken);
  const struct Token *const strTitle = SINGLE_PARSE_ALLOW(StringLiteralToken);
  SINGLE_PARSE_ALLOW(CommaToken);
  const struct Token *const idVisiblityCheckerFunc = SINGLE_PARSE_ALLOW(IdentifierToken);
  SINGLE_PARSE_ALLOW(CommaToken);
  const struct Token *const idTriggerHandlerFunc = SINGLE_PARSE_ALLOW(IdentifierToken);

  struct TokenInfo idParams = {};
  while (true) {
    const struct Token *const nextToken = PEAK_TOKEN();
    if (CloseParenToken == nextToken->type) {
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

// Room idName = idTypeName(intX, intY, stringBody);
// Room idChildTypeName { }
[[nodiscard]] static bool parse_room(const struct TokenInfo *const restrict tokens, size_t *const restrict idx, struct ExpressionInfo *const restrict exprs) {
  const struct Token *const idName = SINGLE_PARSE_ALLOW(IdentifierToken);

  PARSE_BLOCK {
    case OpenBraceToken:
      return parse_typedef(tokens, idx, exprs, &roomTypeName, &roomTypeNames, idName);
    case EqualsToken:
      break;
    DEFAULT_PARSE_ERROR();
  }

  const struct String *idTypeName;
  bool isDerivedType;
  PARSE_BLOCK {
    case RoomKeywordToken:
      idTypeName = &roomTypeName;
      isDerivedType = false;
      break;
    case IdentifierToken:
      const struct Token *const idTypeNameToken = GET_TOKEN();
      if (!is_type_name_known(&roomTypeNames, idTypeNameToken)) {
        UNEXPECTED_TOKEN_ERROR();
        return false;
      }

      idTypeName = &idTypeNameToken->string;
      isDerivedType = true;
      break;
    DEFAULT_PARSE_ERROR();
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

// Screen idName = idTypeName(strBody, array<Action>);
// Screen idName = idTypeName(idBodyFunc, array<Action>);
// Screen idChildTypeName { }
[[nodiscard]] static bool parse_screen(const struct TokenInfo *const restrict tokens, size_t *const restrict idx, struct ExpressionInfo *const restrict exprs) {
  const struct Token *const idName = SINGLE_PARSE_ALLOW(IdentifierToken);

  PARSE_BLOCK {
    case OpenBraceToken:
      return parse_typedef(tokens, idx, exprs, &screenTypeName, &screenTypeNames, idName);
    case EqualsToken:
      break;
    DEFAULT_PARSE_ERROR();
  }

  const struct String *idTypeName;
  bool isDerivedType;
  PARSE_BLOCK {
    case ScreenKeywordToken:
      idTypeName = &screenTypeName;
      isDerivedType = false;
      break;
    case IdentifierToken:
      const struct Token *const idTypeNameToken = GET_TOKEN();
      if (!is_type_name_known(&screenTypeNames, idTypeNameToken)) {
        UNEXPECTED_TOKEN_ERROR();
        return false;
      }

      idTypeName = &idTypeNameToken->string;
      isDerivedType = true;
      break;
    DEFAULT_PARSE_ERROR();
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

    const struct Token *const nextToken = PEAK_TOKEN();
    if (CloseBraceToken == nextToken->type) {
      break;
    } else if (CommaToken == nextToken->type) {
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
      case ActionKeywordToken:
        if (!parse_action(tokens, idx, exprs)) {
          return false;
        }
        break;
      case EnumKeywordToken:
        if (!parse_enumdef(tokens, idx, exprs)) {
          return false;
        }
        break;
      case RoomKeywordToken:
        if (!parse_room(tokens, idx, exprs)) {
          return false;
        }
        break;
      case ScreenKeywordToken:
        if (!parse_screen(tokens, idx, exprs)) {
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
