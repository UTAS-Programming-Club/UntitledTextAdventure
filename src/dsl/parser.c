#include <inttypes.h>
#include <stddef.h>
#include <sys/types.h>

#include "dsl/dsl.h"  // IWYU pragma: associated


// TODO: Fix PEAK_TOKEN causing this if the optional return value is ignored
#pragma GCC diagnostic ignored "-Wunused-value"

#define EMIT_PARSE_ERROR(token, error, ...) {                                          \
	const char *const str = (typeof(str))token->line;                                    \
	const char *end = strchr(str, '\n');                                                 \
	const int strLen = nullptr == end ? (int)strlen(str) : (int)(end - str);             \
	fprintf(stderr, "%s:%" PRIu16 ":%" PRIu16 ": \x1b[0;31merror:\x1b[0m " error "\n",   \
		state->inputPath, token->lineNum + 1, token->colNum + 1 __VA_OPT__(,) __VA_ARGS__  \
	);                                                                                   \
	fprintf(stderr, " %4" PRIu16 " | %.*s\n", token->lineNum + 1, strLen, str);          \
	fprintf(stderr, "      |%*c\x1b[0;32m^\x1b[0m\n", token->colNum + 1, ' ');           \
}

#define ADDITIONAL_TOKENS_ERROR() EMIT_PARSE_ERROR(GET_TOKEN(), "Additional token(s) were expected")
#define UNEXPECTED_TOKEN_ERROR()  EMIT_PARSE_ERROR(GET_TOKEN(), "An unxpected token %s was encountered", get_token_string(GET_TOKEN()->type))

#define DEFAULT_PARSE_ERROR() \
	default:                    \
		UNEXPECTED_TOKEN_ERROR(); \
		return false


#define GET_TOKEN() (state->tokens.tokens + *idx)
#define PEAK_TOKEN()                     \
	state->tokens.tokens + *idx + 1;       \
	if (*idx + 1 >= state->tokens.count) { \
		ADDITIONAL_TOKENS_ERROR();           \
		return false;                        \
	}

#define PARSE_BLOCK          \
	PEAK_TOKEN();              \
	++(*idx);                  \
	switch (GET_TOKEN()->type)

#define REQUIRE_TOKEN(type) \
	PARSE_BLOCK {             \
		case type: break;       \
		DEFAULT_PARSE_ERROR();  \
	}


#define NEW_EXPR(type, ...)                     \
	struct Expression expr = {type, __VA_ARGS__}; \
	if (!add_expr(state, &state->exprs, &expr)) { \
		return false;                               \
	}


DYN_ARRAY_IMPL(StringArray, struct String, string)
DYN_ARRAY_IMPL(ExpressionArray, struct Expression, expr)


static const char *get_token_string(enum TokenType token) {
	switch (token) {
		// Literals
		case BooleanLiteralToken: return "BooleanLiteralToken";
		case IntegerLiteralToken: return "IntegerLiteralToken";
		case StringLiteralToken: return "StringLiteralToken";
		// Type Keywords
		case BooleanTypeToken: return "BooleanTypeToken";
		case IntegerTypeToken: return "IntegerTypeToken";
		case StringTypeToken: return "StringTypeToken";
		case EnumTypeToken: return "EnumTypeToken";
		case ActionTypeToken: return "ActionTypeToken";
		case RoomTypeToken: return "RoomTypeToken";
		case ScreenTypeToken: return "ScreenTypeToken";
		case GameInfoTypeToken: return "GameInfoTypeToken";
		// Keywords
		case IfToken: return "IfToken";
		case WhileToken: return "WhileToken";
		case ForToken: return "ForToken";
		case SwitchToken: return "SwitchToken";
		case CaseToken: return "CaseToken";
		case ContinueToken: return "ContinueToken";
		case BreakToken: return "BreakToken";
		case ReturnToken: return "ReturnToken";
		case IsToken: return "IsToken";
		case NotToken: return "NotToken";
		case NoneToken: return "NoneToken";
		case NamespaceToken: return "NamespaceToken";
		case OverrideToken: return "OverrideToken";
		case ConstToken: return "ConstToken";
		case ThisToken: return "ThisToken";
		// Symbols
		case OpenCurlyToken: return "OpenCurlyToken";
		case CloseCurlyToken: return "CloseCurlyToken";
		case OpenBracketToken: return "OpenBracketToken";
		case CloseBracketToken: return "CloseBracketToken";
		case SemiColonToken: return "SemiColonToken";
		case CommaToken: return "CommaToken";
		case DotToken: return "DotToken";
		case ColonToken: return "ColonToken";
		case PlusToken: return "PlusToken";
		case MinusToken: return "MinusToken";
		case MultiplyToken: return "MultiplyToken";
		case DivideToken: return "DivideToken";
		case ModulusToken: return "ModulusToken";
		case LessThanToken: return "LessThanToken";
		case GreaterThanToken: return "GreaterThanToken";
		case LessThanEqualToken: return "LessThanEqualToken";
		case GreaterThanEqualToken: return "GreaterThanEqualToken";
		case EqualsToken: return "EqualsToken";
		case AssignToken: return "AssignToken";
		case IdentifierToken: return "IdentifierToken";
	}

	unreachable();
}


[[nodiscard]] static bool parse_classdef(struct State *const restrict state, ssize_t *const restrict idx) {
	const struct Token *const restrict idSuperClassName = GET_TOKEN();

	const struct Token *const restrict idClassName = REQUIRE_TOKEN(IdentifierToken);

	PARSE_BLOCK {
		case OpenCurlyToken:
			break;
		case SemiColonToken:
			// TODO: Support definition
		DEFAULT_PARSE_ERROR();
	};

	REQUIRE_TOKEN(CloseCurlyToken);

	const struct Expression expr = {
		ClassDefinitionExpression,
		.name = idClassName,
		.superName = idSuperClassName,
	};

	return add_expr(state, &state->exprs, &expr);
}

// enum keyword already processed in parse
/* enum idEnumName {
 *   idEnumValueName,
 *   ...
 * }
 */
[[nodiscard]] static bool parse_enumdef(struct State *const restrict state, ssize_t *const restrict idx) {
	const struct Token *const restrict idEnumName = REQUIRE_TOKEN(IdentifierToken);
	REQUIRE_TOKEN(OpenCurlyToken);

	struct StringArray idEnumValueNames = {};
	while(true) {
		const struct Token *restrict idEnumValueName = REQUIRE_TOKEN(IdentifierToken);
		if (!add_string(state, &idEnumValueNames, &idEnumValueName->string)) {
			return false;
		}

		const struct Token *restrict nextToken = PEAK_TOKEN();
		if (CommaToken != nextToken->type) {
				break;
		}
		REQUIRE_TOKEN(CommaToken);
	}

	REQUIRE_TOKEN(CloseCurlyToken);

	const struct Expression expr = {
		EnumDefinitionExpression,
		.name = idEnumName,
		.strings = idEnumValueNames
	};

	return add_expr(state, &state->exprs, &expr);
}

[[nodiscard]] bool parse(struct State *const restrict state) {
	ssize_t i = -1;
	ssize_t *idx = &i;

	// namespace idNamespaceName;
	REQUIRE_TOKEN(NamespaceToken);
	const struct Token *const restrict idNamespaceName = REQUIRE_TOKEN(IdentifierToken);
	REQUIRE_TOKEN(SemiColonToken);
	NEW_EXPR(
		NamespaceExpression,
		.name = idNamespaceName
	);

	while(i < state->tokens.count - 1) {
		PARSE_BLOCK {
			case ActionTypeToken:
			case RoomTypeToken:
			case ScreenTypeToken:
				if (!parse_classdef(state, idx)) {
					return false;
				}
				break;
			case EnumTypeToken:
				if (!parse_enumdef(state, idx)) {
					return false;
				}
				break;
			DEFAULT_PARSE_ERROR();
		}
	}
	return true;
}
