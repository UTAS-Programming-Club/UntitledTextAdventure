#include <stdio.h>    // for fprintf, stderr, size_t
#include <string.h>   // for strchr, strlen, strncmp

#include "dsl/dsl.h"  // IWYU pragma: associated

#define EMIT_PARSE_ERROR(token, error, ...) {                                    \
  const char *const str = (typeof(str))token->line;                              \
  const char *end = strchr(str, '\n');                                           \
  const int strLen = nullptr == end ? (int)strlen(str) : (int)(end - str);       \
  fprintf(stderr, "%s:%w16u:%w16u: \u001b[0;31merror:\u001b[0m " error "\n",     \
    inputPath, token->lineNum + 1, token->colNum + 1 __VA_OPT__(,) __VA_ARGS__   \
  );                                                                             \
  fprintf(stderr, " %4w16u | %.*s\n", token->lineNum + 1, strLen, str);          \
  fprintf(stderr, "      |%*c\u001b[0;32m^\u001b[0m\n", token->colNum + 1, ' '); \
}

#define ADDITIONAL_TOKENS_ERROR() EMIT_PARSE_ERROR((*token), "Additional token(s) were expected")
#define UNEXPECTED_TOKEN_ERROR()  EMIT_PARSE_ERROR((*token), "An unxpected token %s was encountered", get_token_string((*token)->type))

#define PARSE_BLOCK            \
  ++(*token);                  \
  if (*token > end) {          \
    ADDITIONAL_TOKENS_ERROR(); \
    return false;              \
  }                            \
                               \
  switch ((*token)->type)

#define SINGLE_PARSE_ALLOW(tokenType) \
  PARSE_BLOCK {                       \
    case tokenType: break;            \
    default:                          \
      UNEXPECTED_TOKEN_ERROR();       \
      return false;                   \
  }                                   \
                                      \
  *token


DYN_ARRAY_IMPL(ExpressionInfo, struct Expression, expr)


// TODO: Move these to params?
static const struct String actionTypeName = { u8"Action", 6 };
static struct StringInfo actionTypeNames = {};


static const char *get_token_string(enum TokenType token) {
  switch (token) {
    case IntegerLiteralToken: return "IntegerLiteralToken";
    case StringLiteralToken:  return "StringLiteralToken";

    case RoomTypeToken:   return "RoomTypeToken";
    case ScreenTypeToken: return "ScreenTypeToken";

    case OpenBraceToken:  return "OpenBraceToken";
    case CloseBraceToken: return "CloseBraceToken";
    case OpenParenToken:  return "OpenParenToken";
    case CloseParenToken: return "CloseParenToken";
    case SemicolonToken:  return "SemicolonToken";
    case ColonToken:      return "ColonToken";
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

static bool is_action_type_name_known(const struct String *restrict const name) {
  for (size_t i = 0; i < actionTypeNames.count; ++i) {
    if (string_equals(actionTypeNames.strings + i, name)) {
      return true;
    }
  }

  return false;
}


// idNewTypeName : idBaseTypeName;
[[nodiscard]] static bool parse_typedec(const struct Token *restrict *const restrict token, const struct Token *const restrict end, struct ExpressionInfo *const restrict exprs) {
  const struct String *const idNewTypeName = &(*token)->string;

  SINGLE_PARSE_ALLOW(ColonToken);
  const struct Token *const idBaseTypeName = SINGLE_PARSE_ALLOW(IdentifierToken);
  if (!string_equals(&actionTypeName, &idBaseTypeName->string)) {
    UNEXPECTED_TOKEN_ERROR();
    return false;
  }

  PARSE_BLOCK {
    case SemicolonToken:
      if (!add_string(&actionTypeNames, idNewTypeName)) {
        return false;
      }

      const struct Expression expr = {
        ActionTypeDeclarationExpression, idBaseTypeName,
        .actionType = { idNewTypeName }
      };
      return add_expr(exprs, &expr);
    default:
      UNEXPECTED_TOKEN_ERROR();
      return false;
  }
}

// idTypeName idName = idTypeName(strTitle, idVisiblityCheckerFunc, idTriggerHandlerFunc);
[[nodiscard]] static bool parse_action(const struct Token *restrict *const restrict token, const struct Token *const restrict end, struct ExpressionInfo *const restrict exprs) {
  const struct String *const idTypeName = &(*token)->string;

  const struct Token *const idName = SINGLE_PARSE_ALLOW(IdentifierToken);
  SINGLE_PARSE_ALLOW(EqualsToken);
  SINGLE_PARSE_ALLOW(IdentifierToken);
  if (!string_equals(idTypeName, &(*token)->string)) {
    UNEXPECTED_TOKEN_ERROR();
    return false;
  }

  SINGLE_PARSE_ALLOW(OpenParenToken);
  const struct Token *const strTitle = SINGLE_PARSE_ALLOW(StringLiteralToken);
  SINGLE_PARSE_ALLOW(CommaToken);
  const struct Token *const idVisiblityCheckerFunc = SINGLE_PARSE_ALLOW(IdentifierToken);
  SINGLE_PARSE_ALLOW(CommaToken);
  const struct Token *const idTriggerHandlerFunc = SINGLE_PARSE_ALLOW(IdentifierToken);
  SINGLE_PARSE_ALLOW(CloseParenToken);

  PARSE_BLOCK {
    case SemicolonToken:
      const bool isDerivedType = !string_equals(&actionTypeName, idTypeName);
      const struct Expression expr = {
        ActionDefinitionExpression, idName,
        .action = {
          strTitle,
          idVisiblityCheckerFunc, idTriggerHandlerFunc,
          idTypeName, isDerivedType
        }
      };
      return add_expr(exprs, &expr);
    default:
      UNEXPECTED_TOKEN_ERROR();
      return false;
  }
}

// Room idName = Room(intX, intY, stringBody);
[[nodiscard]] static bool parse_room(const struct Token *restrict *const restrict token, const struct Token *const restrict end, struct ExpressionInfo *const restrict exprs) {
  const struct Token *const idName = SINGLE_PARSE_ALLOW(IdentifierToken);
  SINGLE_PARSE_ALLOW(EqualsToken);
  SINGLE_PARSE_ALLOW(RoomTypeToken);
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
        .room = { intX, intY, strBody }
      };
      return add_expr(exprs, &expr);
    default:
      UNEXPECTED_TOKEN_ERROR();
      return false;
  }
}

// Screen idName = Screen(strBody, array<Action>);
// Screen idName = Screen(idBodyFunc, array<Action>);
[[nodiscard]] static bool parse_screen(const struct Token *restrict *const restrict token, const struct Token *const restrict end, struct ExpressionInfo *const restrict exprs) {
  const struct Token *const idName = SINGLE_PARSE_ALLOW(IdentifierToken);
  SINGLE_PARSE_ALLOW(EqualsToken);
  SINGLE_PARSE_ALLOW(ScreenTypeToken);
  SINGLE_PARSE_ALLOW(OpenParenToken);

  bool isBodyFunc = true;
  const struct Token *strBody;
  PARSE_BLOCK {
    case StringLiteralToken:
      isBodyFunc = false;
      [[fallthrough]];
    case IdentifierToken:
      strBody = *token;
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

    if ((*token) >= end) {
      ADDITIONAL_TOKENS_ERROR();
      return false;
    }

    if (CloseBraceToken == (*token)[1].type) {
      break;
    } else if (CommaToken == (*token)[1].type) {
      ++(*token);
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
        .screen = { isBodyFunc, strBody, actions }
      };
      return add_expr(exprs, &expr);
    default:
      UNEXPECTED_TOKEN_ERROR();
      return false;
  }
}

[[nodiscard]] bool parse(const struct TokenInfo *const restrict tokens, struct ExpressionInfo *const restrict exprs) {
  if (!add_string(&actionTypeNames, &actionTypeName)) {
    return false;
  }

  const struct Token *token = tokens->tokens;
  const struct Token *const end = token + tokens->count;
  for (; token < end; ++token) {
    switch (token->type) {
      case RoomTypeToken:
        if (!parse_room(&token, end, exprs)) {
          return false;
        }
        continue;
      case ScreenTypeToken:
        if (!parse_screen(&token, end, exprs)) {
          return false;
        }
        continue;
      case IdentifierToken:
        if (is_action_type_name_known(&token->string)) {
          if (!parse_action(&token, end, exprs)) {
            return false;
          }
        } else if (!parse_typedec(&token, end, exprs)) {
          return false;
        }

        continue;
      default:
        EMIT_PARSE_ERROR(token, "An unxpected token %s was encountered", get_token_string(token->type));
        return false;
    }
  }

  return true;
}
