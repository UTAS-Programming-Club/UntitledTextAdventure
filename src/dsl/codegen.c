#include "dsl/dsl.h"  // IWYU pragma: associated

#define FSTRING(string) (int)string.len, string.str

[[nodiscard]] bool codegen(const struct State *const restrict state, const char *const restrict headerPath, const char *const restrict sourcePath, const char *const restrict extensionName) {
	for (size_t i = 0; i < state->exprs.count; ++i) {
		const struct Expression *const restrict expr = state->exprs.exprs + i;
		switch (expr->type) {
		case NamespaceExpression:
			printf("NAMESPACE: %.*s\n", FSTRING(expr->name->string));
			break;
		case EnumDefinitionExpression:
			printf("ENUM: %.*s(%.*s", FSTRING(expr->name->string), FSTRING(expr->strings.strings[0]));
			for (size_t i = 1; i < expr->strings.count; ++i) {
				printf(", %.*s", FSTRING(expr->strings.strings[i]));
			}
			puts(")");
			break;
		}
	}

	return false;
}
