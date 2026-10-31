#include <inttypes.h>

#include "dsl/dsl.h"  // IWYU pragma: associated

#define PRINT_TOKEN(fmt, ...)                                                                                        \
  printf("%02" PRIu16 ":%02" PRIu16 ": " fmt "\n", token->lineNum + 1, token->colNum + 1 __VA_OPT__(,) __VA_ARGS__); \
  break

[[nodiscard]] bool parse(struct State *const restrict state) {
  for (size_t i = 0; i < state->tokens.count; ++i) {
    const struct Token *const restrict token = state->tokens.tokens + i;
    switch (token->type) {
      // Literals
      case BooleanLiteralToken:
        PRINT_TOKEN("%s", token->boolean ? "true" : "false");
      case IntegerLiteralToken:
        PRINT_TOKEN("%" PRIu64, token->integer);
      case StringLiteralToken:
      case IdentifierToken:
        PRINT_TOKEN("%.*s", (int)token->string.len, token->string.str);

      // Type Keywords
      case BooleanTypeToken:
        PRINT_TOKEN("bool");
      case IntegerTypeToken:
        PRINT_TOKEN("%s", token->boolean ? "int" : "uint");
      case StringTypeToken:
        PRINT_TOKEN("string");
      case EnumTypeToken:
        PRINT_TOKEN("enum");
      case ActionTypeToken:
        PRINT_TOKEN("Action");
      case RoomTypeToken:
        PRINT_TOKEN("Room");
      case ScreenTypeToken:
        PRINT_TOKEN("Screen");
      case GameInfoTypeToken:
        PRINT_TOKEN("GameInfo");

      // Keywords
      case IfToken:
        PRINT_TOKEN("if");
      case WhileToken:
        PRINT_TOKEN("while");
      case ForToken:
        PRINT_TOKEN("for");
      case SwitchToken:
        PRINT_TOKEN("switch");
      case CaseToken:
        PRINT_TOKEN("case");
      case ContinueToken:
        PRINT_TOKEN("continue");
      case BreakToken:
        PRINT_TOKEN("break");
      case ReturnToken:
        PRINT_TOKEN("return");
      case IsToken:
        PRINT_TOKEN("is");
      case NotToken:
        PRINT_TOKEN("not");
      case NoneToken:
        PRINT_TOKEN("none");
      case NamespaceToken:
        PRINT_TOKEN("namespace");
      case OverrideToken:
        PRINT_TOKEN("override");
      case ConstToken:
        PRINT_TOKEN("const");
      case ThisToken:
        PRINT_TOKEN("this");

      // Symbols
      case OpenCurlyToken:
        PRINT_TOKEN("{");
      case CloseCurlyToken:
        PRINT_TOKEN("}");
      case OpenBracketToken:
        PRINT_TOKEN("(");
      case CloseBracketToken:
        PRINT_TOKEN(")");
      case SemiColonToken:
        PRINT_TOKEN(";");
      case CommaToken:
        PRINT_TOKEN(",");
      case DotToken:
        PRINT_TOKEN(".");
      case ColonToken:
        PRINT_TOKEN(":");
      case PlusToken:
        PRINT_TOKEN("+");
      case MinusToken:
        PRINT_TOKEN("-");
      case MultiplyToken:
        PRINT_TOKEN("*");
      case DivideToken:
        PRINT_TOKEN("/");
      case ModulusToken:
        PRINT_TOKEN("%%");
      case LessThanToken:
        PRINT_TOKEN("<");
      case GreaterThanToken:
        PRINT_TOKEN(">");
      case LessThanEqualToken:
        PRINT_TOKEN("<=");
      case GreaterThanEqualToken:
        PRINT_TOKEN(">=");
      case EqualsToken:
        PRINT_TOKEN("==");
      case AssignToken:
        PRINT_TOKEN("=");
    }
  }
  return false;
}
