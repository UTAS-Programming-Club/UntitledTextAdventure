#include <stdio.h>    // for fprintf, stderr
#include <string.h>   // for strchr, strlen

#include "dsl/dsl.h"  // IWYU pragma: associated

#define EMIT_PARSE_ERROR(token, error, ...) {                                    \
  const char *str = (const char *)token->line;                                   \
  const char *end = strchr(str, '\n');                                           \
  const int strLen = nullptr == end ? (int)strlen(str) : (int)(end - str);       \
  fprintf(stderr, "%s:%w16u:%w16u: \u001b[0;31merror\u001b[0m: " error "\n",     \
    inputPath, token->lineNum + 1, token->colNum + 1 __VA_OPT__(,) __VA_ARGS__   \
  );                                                                             \
  fprintf(stderr, " %4w16u | %.*s\n", token->lineNum + 1, strLen, str);          \
  fprintf(stderr, "      |%*c\u001b[0;32m^\u001b[0m\n", token->colNum + 1, ' '); \
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


DYN_ARRAY_IMPL(ExpressionInfo, struct Expression, expr)


static const struct String actionTypeName = { u8"Action", 6 };
static const char8_t *const actionTypeNames[] = { actionTypeName.str };
static const size_t actionTypeNameCount = sizeof actionTypeNames / sizeof *actionTypeNames;


static const char *get_token_string(enum TokenType token) {
  switch (token) {
    case IntegerLiteralToken: return "IntegerLiteralToken";
    case StringLiteralToken: return "StringLiteralToken";

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

static bool isActionTypeNameKnown(const struct String *restrict const name) {
  for (size_t i = 0; i < actionTypeNameCount; ++i) {
    if (0 == strncmp((const char *)actionTypeNames[i], (const char *)name->str, name->strLen)) {
      return true;
    }
  }

  return false;
}

[[nodiscard]] bool parse(const struct TokenInfo *const restrict tokens, struct ExpressionInfo *const restrict exprs) {
  const struct Token *token = tokens->tokens;
  const struct Token *const end = token + tokens->count;
  const struct String *idActionTypeName;
  for (; token < end; ++token) {
    switch (token->type) {
      case RoomTypeToken: goto room;
      case ScreenTypeToken: goto screen;
      case IdentifierToken:
        if (isActionTypeNameKnown(&token->string)) {
          idActionTypeName = &token->string;
          goto action;
        }
        [[fallthrough]];
      default:
        UNEXPECTED_TOKEN_ERROR();
        return false;
    }

  // idActionTypeName idName = idActionTypeName(strTitle, idVisiblityCheckerFunc, idTriggerHandlerFunc);
action:
    const struct Token *idName = SINGLE_PARSE_ALLOW(IdentifierToken);
    SINGLE_PARSE_ALLOW(EqualsToken);
    SINGLE_PARSE_ALLOW(IdentifierToken);
    if (0 != strncmp((const char *)idActionTypeName->str, (const char *)token->string.str, token->string.strLen)) {
      UNEXPECTED_TOKEN_ERROR();
      return false;
    }

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
        bool isDerivedType = 0 != strncmp((const char *)actionTypeName.str, (const char *)idActionTypeName->str, idActionTypeName->strLen);
        struct Expression expr = {
          ActionDefintionExpression, idName,
          .action = {
            strTitle,
            idVisiblityCheckerFunc, idTriggerHandlerFunc,
            idActionTypeName, isDerivedType
          }
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
