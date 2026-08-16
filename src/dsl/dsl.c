#include <ctype.h>     // for isalnum, isdigit, isspace
#include <fcntl.h>     // for O_RDONLY, open
#include <inttypes.h>  // for PRIu16, uint16_t, uint_fast8_t
#include <stdio.h>     // for size_t, fprintf, stderr, fputs, FILE, fclose, fopen, fputc, snprintf, printf
#include <stdlib.h>    // for free, EXIT_FAILURE, realloc, EXIT_SUCCESS, malloc
#include <string.h>    // for memcpy, strstr, strncmp, strcmp
#include <sys/mman.h>  // for MAP_FAILED, MAP_PRIVATE, PROT_READ, mmap, munmap
#include <sys/stat.h>  // for stat, fstat
#include <uchar.h>     // for char8_t
#include <unistd.h>    // for close

const char *programName;
const char *inputPath;

#define EMIT_PROG_ERROR(error, ...) \
	fprintf(stderr, "%s: \x1b[0;31merror\x1b[0m: " error "\n", programName __VA_OPT__(,) __VA_ARGS__)

#define DYN_ARRAY_DEF(typeName, baseTypeName, varName) struct typeName {																							\
		baseTypeName *varName ## s;																																											 	\
		size_t count;																																																		 	\
		size_t length;																																																		\
	};																																																									\
																																																											\
	[[nodiscard]] static bool add_ ## varName(struct typeName *const varName ## s, const baseTypeName *const varName) {	\
	if (varName ## s->count + 1 >= varName ## s->length) {																															\
			size_t newLen = 2 * varName ## s->count;																																				\
			if (0 == newLen) {																																															\
				newLen = 8;																																																	 	\
			}																																																							 	\
																																																											\
			baseTypeName *newArr = realloc(varName ## s->varName ## s, newLen * sizeof *varName ## s->varName ## s);				\
			if (nullptr == newArr) {																																												\
				EMIT_PROG_ERROR("An unrecoverable error occurred");																														\
				return false;																																																	\
			}																																																								\
																																																											\
			varName ## s->length = newLen;																																									\
			varName ## s->varName ## s = newArr;																																						\
		}																																																									\
																																																											\
		memcpy(varName ## s->varName ## s + varName ## s->count, varName, sizeof *varName);																\
		++varName ## s->count;																																														\
		return true;																																																			\
	}

// TODO: Restore full error line reporting, also make sure unicode prints then (doesn't now unless only 1 byte)
// TODO: Fix exprStart not being set properly
#define EMIT_LEX_ERROR()\
	EMIT_PROG_ERROR("%s:%" PRIu16 ":%td: Unexpected character: %c", inputPath, *lineNum + 1, *file - exprStart + 1, **file)


#define NEW_STATIC_STRING(str) { u8 ## str, sizeof u8 ## str - 1 }
#define NEW_TOKEN_STRING() (const struct String){ tokenStart, (size_t)(*file - tokenStart) }
struct String {
	const char8_t *str;
	size_t strLen;
};
DYN_ARRAY_DEF(StringArray, struct String, string)

[[nodiscard]] static bool string_starts_with(
	const struct String *const restrict str1, const struct String *const restrict str2
) {
	return 0 == strncmp((const char *)str1->str, (const char *)str2->str, str2->strLen);
}

[[nodiscard]] static bool string_equals(
	const struct String *const restrict str1, const struct String *const restrict str2
) {
	if (str1->strLen != str2->strLen) {
		return false;
	}

	return string_starts_with(str1, str2);
}

// TODO: Return index and use to simplify get_ functions
[[nodiscard]] static bool string_exists(
	const struct StringArray *const restrict strings, const struct String *const restrict string
) {
	for (size_t i = 0; i < strings->count; ++i) {
		if (string_equals(strings->strings + i, string)) {
			return true;
		}
	}

	return false;
}


struct Enum {
	struct String name;
	struct StringArray valueNames;
};
DYN_ARRAY_DEF(EnumArray, struct Enum, enumType)

static struct EnumArray Enums = {};

[[nodiscard]] static const struct Enum *get_enum(const struct String *const enumName) {
	for (size_t i = 0; i < Enums.count; ++i) {
		const struct Enum *const enumType = Enums.enumTypes + i;
		if (string_equals(&enumType->name, enumName)) {
			return enumType;
		}
	}

	return nullptr;
}

[[nodiscard]] static bool enum_value_exists(
	const struct String *const restrict enumName, const struct String *const restrict enumValueName
) {
	const struct Enum *const enumType = get_enum(enumName);
	if (nullptr == enumType) {
		return false;
	}

	return string_exists(&enumType->valueNames, enumValueName);
}


// TODO: Merge BooleanType & IntegerType (value types)?
enum FieldType {
	ArrayType,           // ItemType[] where ItemType is any Type other than ArrayType or MethodType, only used for fields/function parameters
	BooleanType,         // bool in both dsl and C,                                                   only used for function return types
	EnumType,            // enum EnumName in both dsl and C,                                          only used for fields/function parameters
	IntegerType,         // One of IntegerTypes in both dsl and C,                                    only used for fields/function parameters
	MethodType,          // ReturnType Name() where ReturnType is BooleanType or StringType,          only used for fields/function parameters
	StringType,          // string in dsl and const char * in C,                                      only used for fields/function parameters
	StringGeneratorType, // string and (method) in dsl, const char * and const char *(*)(const struct GameInfo *const) in C, used for a pre defined function parameter
	StructType           // Action, Room and Screen in dsl, same with struct prefix in C,             only used for fields/function parameters
};

struct Field {
	enum FieldType type;
	struct String name; // If type is StructType then only set if parent Type.isDerivedType

	struct String internalTypeName; // Only set if type or arrayBaseType is IntegerType (integerTypeName), or type is MethodType (returnTypeName)
	union {
		struct {
			enum FieldType baseType;
			const struct Type *structBaseType; // Only set if arrayBaseType is StructType
		} array; // Only set if type is ArrayType
		bool methodConstGameInfo; // Only set if type is MethodName
	};
};
DYN_ARRAY_DEF(FieldArray, struct Field, field)

[[nodiscard]] static const struct Field *get_field(
	const struct FieldArray *const restrict fields, const struct String *const restrict fieldName
) {
	for (size_t i = 0; i < fields->count; ++i) {
		const struct Field *field = fields->fields + i;
		if (string_equals(&field->name, fieldName)) {
			return field;
		}
	}

	return nullptr;
}


enum StructType {
	ActionStructType,
	RoomStructType,
	ScreenStructType
};

struct Type {
	enum StructType type;
	struct String name;
	// TODO: Remove?
	const struct String *capitalBaseTypeName;
	struct FieldArray fields;

	bool isDerivedType;
	struct StringArray overridenMethodNames; // Only set if isDerivedType
};
DYN_ARRAY_DEF(TypeArray, struct Type, type)

static struct TypeArray Types = {};

[[nodiscard]] static const struct Type *get_type(const struct String *const typeName) {
	for (size_t i = 0; i < Types.count; ++i) {
		const struct Type *type = Types.types + i;
		if (string_equals(&type->name, typeName)) {
			return type;
		}
	}

	return nullptr;
}


struct Variable {
	struct String name;
	enum StructType type;
};
DYN_ARRAY_DEF(VariableArray, struct Variable, variable)

static struct VariableArray Variables = {};

[[nodiscard]] static const struct Variable *get_variable(const struct String *const variableName) {
	for (size_t i = 0; i < Variables.count; ++i) {
		const struct Variable *const variable = Variables.variables + i;
		if (string_equals(&variable->name, variableName)) {
			return variable;
		}
	}

	return nullptr;
}


static const struct String ActionCapitalTypeName = NEW_STATIC_STRING("ACTION");
static struct Type ActionType = { ActionStructType, NEW_STATIC_STRING("Action"), &ActionCapitalTypeName };
static const struct Field ActionTitle = { StringType };
static const struct Field ActionIsVisible = { MethodType, NEW_STATIC_STRING("IsVisible"), .methodConstGameInfo = true };
static const struct Field ActionHandleAction = { MethodType, NEW_STATIC_STRING("HandleAction") };

static const struct String RoomCapitalTypeName = NEW_STATIC_STRING("ROOM");
static struct Type RoomType = { RoomStructType, NEW_STATIC_STRING("Room"), &RoomCapitalTypeName };
static const struct Field RoomX = { IntegerType };
static const struct Field RoomY = { IntegerType };
static const struct Field RoomBody = { StringType };

static const struct String ScreenCapitalTypeName = NEW_STATIC_STRING("SCREEN");
static struct Type ScreenType = { ScreenStructType, NEW_STATIC_STRING("Screen"), &ScreenCapitalTypeName };
static const struct Field ScreenBody = { StringGeneratorType };
static const struct Field ScreenActions = { ArrayType, .array = { StructType, &ActionType } };

[[nodiscard]] static bool setup_type_arrays() {
	return add_field(&ActionType.fields, &ActionTitle) &&
				 add_field(&ActionType.fields, &ActionIsVisible) &&
				 add_field(&ActionType.fields, &ActionHandleAction) &&
				 add_type(&Types, &ActionType) &&

				 add_field(&RoomType.fields, &RoomX) && add_field(&RoomType.fields, &RoomY) &&
				 add_field(&RoomType.fields, &RoomBody) && add_type(&Types, &RoomType) &&

				 add_field(&ScreenType.fields, &ScreenBody) &&
				 add_field(&ScreenType.fields, &ScreenActions) &&
				 add_type(&Types, &ScreenType);
}


static const struct String IntegerTypes[] = {
	NEW_STATIC_STRING("int8_t"),  NEW_STATIC_STRING("int16_t"),  NEW_STATIC_STRING("int32_t"),
	NEW_STATIC_STRING("uint8_t"), NEW_STATIC_STRING("uint16_t"), NEW_STATIC_STRING("uint32_t")
};
static const size_t IntegerTypeCount = sizeof IntegerTypes / sizeof *IntegerTypes;

[[nodiscard]] static bool type_is_integer(const struct String *type) {
	for (size_t i = 0; i < IntegerTypeCount; ++i) {
		if (string_equals(IntegerTypes + i, type)) {
			return true;
		}
	}

	return false;
}


// TODO: Ensure this supports unicode
#define process_match(match) process_match(file, sizeof u8 ## match - 1, u8 ## match)
[[nodiscard]] static bool (process_match)(
	const char8_t *restrict *const restrict file, size_t strLen, const char8_t match[const restrict static strLen]
) {
	if (0 != strncmp((const char *)*file, (const char *)match, strLen)) {
		return false;
	}

	*file += strLen;
	return true;
}

// TODO: Support unicode?
#define process_spaces() process_spaces(file, lineNum)
static void (process_spaces)(const char8_t *restrict *const restrict file, uint16_t *const restrict lineNum) {
	bool inSingleLineComment = false;
	bool inMultiLineComment = false;
	for (; u8'\0' != **file; ++*file) {
		if (u8'\n' == **file) {
			inSingleLineComment = false;
			++*lineNum;
		// Single-line comments continue until end of line so don't bother checking anything else
		} else if (inSingleLineComment) {
			continue;
		// Check for end of multi-line comment
		} else if (u8'*' == **file && u8'/' == (*file)[1]) {
			inMultiLineComment = false;
			++*file;
			continue;
		// Multi-line comments continue until */ so don't bother checking anything else
		} else if (inMultiLineComment) {
			continue;
		// Check for start of multi-line comment
		} else if (u8'/' == **file && u8'*' == (*file)[1]) {
			inMultiLineComment = true;
			++*file;
			continue;
		// Check for start of single-line comment
		} else if (u8'/' == **file && u8'/' == (*file)[1]) {
			inSingleLineComment = true;
			++*file;
			continue;
		}

		if (!isspace(**file)) {
				break;
		}
	}
}

// TODO: Support unicode
#define process_identifier() process_identifier(file)
[[nodiscard]] static bool (process_identifier)(const char8_t *restrict *const restrict file) {
	bool ranOnce = false;
	for (; u8'\0' != **file; ++*file) {
		if (u8'_' != **file && !isalnum(**file)) {
			break;
		}

		ranOnce = true;
	}

	return ranOnce;
}


#define process_argument(expectedType, argument) process_argument(file, expectedType, &argument)
[[nodiscard]] static bool (process_argument)(
	const char8_t *restrict *const restrict file, const struct Field *const restrict expectedType,
	struct String *const restrict argument
);

#define process_array(itemType, arrayItems) process_array(file, lineNum, itemType, &arrayItems)
[[nodiscard]] static bool (process_array)(
	const char8_t *restrict *const restrict file, uint16_t *const restrict lineNum,
	const struct Field *const restrict itemType, struct StringArray *const restrict arrayItems
) {
	if (!process_match("{")) {
		return false;
	}

	for (uint_fast8_t i = 0; i < 255 && u8'\0' != **file; ++i) {
		process_spaces();

		if (i != 0) {
			if (!process_match(",")) {
				break;
			}

			process_spaces();
		}

		struct String arrayItem = {};
		if (!process_argument(itemType, arrayItem)) {
			return false;
		}
		if (!add_string(arrayItems, &arrayItem)) {
			return false;
		}
	}

	if (!process_match("}")) {
		return false;
	}

	return true;
}

#define process_integer() process_integer(file)
[[nodiscard]] static bool (process_integer)(const char8_t *restrict *const restrict file) {
	bool ranOnce = false;
	for (; u8'\0' != **file; ++*file) {
		if (!isdigit(**file)) {
			break;
		}

		ranOnce = true;
	}

	return ranOnce;
}

#define process_method_body() process_method_body(file, lineNum)
[[nodiscard]] static bool (process_method_body)(
	const char8_t *restrict *const restrict file, uint16_t *const restrict lineNum
) {
	uint_fast8_t depth = 0;
	for (; u8'\0' != **file; ++*file) {
		if (u8'\n' == **file) {
			++*lineNum;
		} else if (u8'{' == **file) {
			if (255 == depth) {
				return false;
			}
			++depth;
		} else if (u8'}' == **file) {
			if (0 < depth) {
				--depth;
			} else {
				++*file;
				return true;
			}
		}
	}

	return false;
}

#define process_string() process_string(file)
[[nodiscard]] static bool (process_string)(const char8_t *restrict *const restrict file) {
	if (!process_match("\"")) {
		return false;
	}

	for (; u8'\0' != **file; ++*file) {
		if (u8'"' == **file) {
			break;
		}
	}

	if (!process_match("\"")) {
		return false;
	}

	return true;
}

[[nodiscard]] static bool (process_argument)(
	const char8_t *restrict *const restrict file, const struct Field *const restrict expectedType,
	struct String *const restrict argument
) {
	const char8_t *const tokenStart = *file;
	const enum FieldType type = ArrayType == expectedType->type ? expectedType->array.baseType : expectedType->type;

	bool isVariableType = false;
	switch (type) {
		case ArrayType:
		case BooleanType:
		case MethodType:
			return false;
		case IntegerType:
			if (!process_integer()) {
				return false;
			}
			break;
		// TODO: Skip string if method exists
		case StringGeneratorType:
		// TODO: Support multi line strings
		case StringType:
			if (!process_string()) {
				return false;
			}
			break;
		case StructType:
			isVariableType = true;
			[[fallthrough]];
		case EnumType:
			if (!process_identifier()) {
				return false;
			}
			break;
	}

	*argument = NEW_TOKEN_STRING();

	if (EnumType == expectedType->type) {
		if (!enum_value_exists(&expectedType->internalTypeName, argument)) {
			return false;
		}
	} else if (isVariableType) {
		const struct Variable *const variable = get_variable(argument);
		if (nullptr == variable || nullptr == expectedType->array.structBaseType ||
				variable->type != expectedType->array.structBaseType->type) {
			return false;
		}
	}

	return true;
}


#define FSTRING(string) (int)(string)->strLen, (string)->str

// enum keyword already processed in transpile
/* enum EnumType {
 *   EnumValue,
 *   ...
 * }
 */
[[nodiscard]] static bool transpile_enum(
	const char8_t *restrict *const restrict file, FILE *const restrict fh, const struct String *const restrict namespace,
	uint16_t *const restrict lineNum
) {
	process_spaces();

	const char8_t *tokenStart = *file;
	if (!process_identifier()) {
		return false;
	}
	const struct String name = NEW_TOKEN_STRING();
	if (nullptr != get_enum(&name)) {
		return false;
	}

	process_spaces();

	if (!process_match("{")) {
		return false;
	}

	struct StringArray valueNames = {};
	for (uint_fast8_t i = 0; i < 255 && u8'\0' != **file; ++i) {
		process_spaces();

		if (i != 0) {
			if (!process_match(",")) {
				break;
			}

			process_spaces();
		}

		tokenStart = *file;
		if (!process_identifier()) {
			break;
		}
		const struct String valueName = NEW_TOKEN_STRING();

		if (!add_string(&valueNames, &valueName)) {
			return false;
		}
	}

	if (!process_match("}")) {
		return false;
	}

	fprintf(fh, "enum %.*s_%.*s {\n", FSTRING(namespace), FSTRING(&name));
	for (size_t i = 0; i < valueNames.count; ++i) {
		fprintf(fh, "  %.*s_%.*s_%.*s,\n", FSTRING(namespace), FSTRING(&name), FSTRING(valueNames.strings + i));
	}
	fputs("};\n\n", fh);

	const struct Enum enumType = { name, valueNames };
	return add_enumType(&Enums, &enumType);
}

// override keyword already processed in transpile_type
/* override bool MethodName([const] Gameinfo info) {
 *   CFunctionBody
 * }
 */
[[nodiscard]] static bool transpile_method(
	const char8_t *restrict *const restrict file, FILE *const restrict fh, FILE *const restrict fc,
	const struct String *const restrict namespace, uint16_t *const restrict lineNum,
	const struct Type *const restrict baseType, const struct String *const restrict typeName,
	struct StringArray *const restrict methods
) {
	process_spaces();

	// TODO: Support returning strings for GetBody
	if (!process_match("bool")) {
		return false;
	}

	process_spaces();

	const char8_t *tokenStart = *file;
	if (!process_identifier()) {
		return false;
	}
	const struct String name = NEW_TOKEN_STRING();

	const struct Field *const method = get_field(&baseType->fields, &name);
	if (nullptr == method || MethodType != method->type) {
		return false;
	}

	process_spaces();

	if (!process_match("(")) {
		return false;
	}

	process_spaces();

	if (method->methodConstGameInfo) {
		if (!process_match("const")) {
			return false;
		}

		process_spaces();
	}

	if (!process_match("GameInfo")) {
		return false;
	}

	process_spaces();

	if (!process_match("info")) {
		return false;
	}

	process_spaces();

	if (!process_match(")")) {
		return false;
	}

	process_spaces();

	if (!process_match("{")) {
		return false;
	}

	process_spaces();

	tokenStart = *file;
	if (!process_method_body()) {
		return false;
	}
	struct String body = NEW_TOKEN_STRING();

	if (!add_string(methods, &name)) {
		return false;
	}

	fprintf(fc, "\
static bool %.*s_%.*s_%.*s(",
		FSTRING(namespace), FSTRING(typeName), FSTRING(&name));
	if (method->methodConstGameInfo) {
		fputs("const ", fc);
	}
	fprintf(fc, "struct GameInfo *const info, const struct %.*s *const base) {\n\
		const struct %.*s_%.*s *const this = (const struct %.*s_%.*s *const)base;\n\
		(void)this;\n\
\n\
		",
		FSTRING(&baseType->name), FSTRING(namespace), FSTRING(typeName), FSTRING(namespace), FSTRING(typeName));

	static const struct String infoAccess = NEW_STATIC_STRING("info.");
	static const struct String thisAccess = NEW_STATIC_STRING("this.");

	// - 1 to skip closing }
	--body.strLen;
	while (0 < body.strLen) {
		size_t writtenCount;
		if (string_starts_with(&body, &infoAccess)) {
			fputs("info->", fc);
			writtenCount = infoAccess.strLen;
		} else if (string_starts_with(&body, &thisAccess)) {
			const char8_t *const fieldNameStr = body.str + thisAccess.strLen;
			const char8_t *str = fieldNameStr;
			if (!(process_identifier)(&str)) {
				return false;
			}
			const struct String fieldName = { fieldNameStr, (size_t)(str - fieldNameStr) };

			if (nullptr != get_field(&baseType->fields, &fieldName)) {
				fputs("base", fc);
			// If not base field then assume part of child
			} else {
				fputs("this", fc);
			}

			fprintf(fc, "->");
			writtenCount = thisAccess.strLen;
		} else {
			fputc(*body.str, fc);
			writtenCount = 1;
		}

		body.str += writtenCount;
		body.strLen -= writtenCount;
	}

	fputs("\n}\n\n", fc);

	return true;
}

// BaseType = Action | Room | Screen
/* BaseType Type {
 *   FieldType FieldName;
 *   ...
 *
 *   override bool MethodName(...) { ... }
 *   ...
 * }
 */
[[nodiscard]] static bool transpile_type(
	const char8_t *restrict *const restrict file, FILE *const restrict fh, FILE *const restrict fc,
	const struct String *const restrict namespace, uint16_t *const restrict lineNum,
	const struct Type *const restrict baseType, const struct String *const restrict name
) {
	if (nullptr != get_type(name)) {
		// TODO: Add error
		return false;
	}

	struct FieldArray fields = {};
	struct StringArray overridenMethodNames = {};
	bool finishedFields = false;
	for (process_spaces(); u8'\0' != **file; process_spaces()) {
		bool isMethod = process_match("override");;
		finishedFields |= isMethod;
		if (finishedFields) {
			if (!isMethod) {
				break;
			}

			if (!transpile_method(file, fh, fc, namespace, lineNum, baseType, name, &overridenMethodNames)) {
				goto type_failure;
			}

			continue;
		}

		bool isEnum = process_match("enum");
		if (isEnum) {
			process_spaces();
		}

		const char8_t *tokenStart = *file;
		if (!process_identifier()) {
			break;
		}
		const struct String fieldTypeName = NEW_TOKEN_STRING();

		// TODO: Allow bool fields
		// TODO: Allow string fields
		// TODO: Allow array fields of bools, integers and strings
		enum FieldType type;
		const struct Type *const structBaseType = get_type(&fieldTypeName);
		if (nullptr != structBaseType) {
			if (structBaseType->isDerivedType) {
				return false;
			}
			type = StructType;
		} else if (isEnum) {
			type = EnumType;
		} else if (type_is_integer(&fieldTypeName)) {
			type = IntegerType;
		} else {
			goto type_failure;
		}

		process_spaces();

		tokenStart = *file;
		if (!process_identifier()) {
			goto type_failure;
		}
		const struct String fieldName = NEW_TOKEN_STRING();

		process_spaces();
		if (!process_match(";")) {
			goto type_failure;
		}

		struct Field field = {
			type, fieldName, .internalTypeName = fieldTypeName, .array = { .structBaseType = structBaseType }
		};
		if (!add_field(&fields, &field)) {
			goto type_failure;
		}
	}

	process_spaces();
	if (!process_match("}")) {
		goto type_failure;
	}

	struct Type type = { baseType->type, *name, baseType->capitalBaseTypeName, fields, true, overridenMethodNames };
	if (!add_type(&Types, &type)) {
		goto type_failure;
	}

	fprintf(fh, "struct %.*s_%.*s {\n"
							"  const struct %.*s base;\n\n",
					FSTRING(namespace), FSTRING(name),
					FSTRING(&baseType->name)
	);

	for (size_t i = 0; i < fields.count; ++i) {
		const struct Field *const field = fields.fields + i;

		fputs("  const ", fh);

		switch (field->type) {
			case ArrayType:
			case BooleanType:
			case MethodType:
			case StringGeneratorType:
				goto type_failure;
			case StructType:
				fprintf(fh, "struct %.*s *const %.*s;\n", FSTRING(&field->internalTypeName), FSTRING(&field->name));
				break;
			case EnumType:
				fprintf(fh, "enum %.*s_", FSTRING(namespace));
				[[fallthrough]];
			case IntegerType:
				fprintf(fh, "%.*s %.*s;\n", FSTRING(&field->internalTypeName), FSTRING(&field->name));
				break;
			case StringType:
				fprintf(fh, "char *%.*s;\n", FSTRING(&field->name));
				break;
		}
	}

	fputs("};\n\n", fh);
	// TODO: Add typedef?

	return true;

type_failure:
	free(fields.fields);

	return false;
}

[[nodiscard]] static bool transpile_arguments(
	const char8_t *restrict *const restrict file, FILE *const restrict fc, const struct String *const restrict namespace,
	uint16_t *const restrict lineNum, const struct Type *const restrict type,
	struct StringArray *const restrict arguments
) {
	for (size_t i = 0; i < type->fields.count; ++i) {
		const struct Field *field = type->fields.fields + i;
		if (MethodType == field->type) {
			if (!add_string(arguments, &field->name)) {
				return false;
			}
			continue;
		}

		process_spaces();

		if (type->isDerivedType || i != 0) {
			if (!process_match(",")) {
				return false;
			}

			process_spaces();
		}

		struct String argument = {};
		switch (field->type) {
			case ArrayType:
				struct StringArray arrayItems = {};
				if (!process_array(field, arrayItems)) {
					free(arrayItems.strings);
					return false;
				}

				argument.strLen = (size_t)snprintf(nullptr, 0, "array%" PRIu16 "_%zu", *lineNum, i);
				if (0 > argument.strLen) {
					free(arrayItems.strings);
					return false;
				}
				argument.str = malloc(argument.strLen + 1);
				if (nullptr == argument.str) {
					free(arrayItems.strings);
					return false;
				}
				argument.strLen = (size_t)snprintf(
					(char *)argument.str, argument.strLen + 1, "array%" PRIu16 "_%zu", *lineNum + 1, i
				);
				if (0 > argument.strLen) {
					free((void *)argument.str);
					free(arrayItems.strings);
					return false;
				}

				fputs("static const ", fc);
				switch (field->array.baseType) {
					case ArrayType:
					case BooleanType:
					case EnumType:
					case IntegerType:
					case MethodType:
					case StringType:
					case StringGeneratorType:
						free((void *)argument.str);
						free(arrayItems.strings);
						return false;
					case StructType:
						fprintf(fc, "%.*s", FSTRING(&field->array.structBaseType->name));
						break;
				}
				fprintf(fc, " %.*s[] = { ",  FSTRING(&argument));
				for (size_t j = 0; j < arrayItems.count; ++j) {
					if (0 != j) {
						fputs(", ", fc);
					}

					fprintf(fc, "USE_%.*s(%.*s_%.*s)",
						FSTRING(field->array.structBaseType->capitalBaseTypeName), FSTRING(namespace),
						FSTRING(arrayItems.strings + j)
					);
				}
				fputs(" };\n", fc);

				free(arrayItems.strings);
				break;
			default:
				if (!process_argument(field, argument)) {
					return false;
				}
				break;
		}

		if (!add_string(arguments, &argument)) {
			return false;
		}

		if (StringGeneratorType == field->type) {
			argument = (struct String)NEW_STATIC_STRING("backend_default_screen_body_generator");
			if (!add_string(arguments, &argument)) {
				return false;
			}
		}
	}

	return true;
}

// BaseType = Action | Room | Screen
// BaseType VariableName = Type([... [, ... [...]]]);
[[nodiscard]] static bool transpile_variable(
	const char8_t *restrict *const restrict file, FILE *const restrict fh, FILE *const restrict fc,
	const struct String *const restrict namespace, uint16_t *const restrict lineNum,
	const struct Type *const restrict baseType, const struct String *const restrict name
) {
	process_spaces();

	const char8_t *tokenStart = *file;
	if (!process_identifier()) {
		return false;
	}
	const struct String typeName = NEW_TOKEN_STRING();
	const struct Type *type = get_type(&typeName);
	if (nullptr == type) {
		// TODO: Add error
		return false;
	}

	process_spaces();
	if (!process_match("(")) {
		return false;
	}

	bool status = false;
	struct StringArray arguments = {};
	if (!transpile_arguments(file, fc, namespace, lineNum, baseType, &arguments)) {
		goto variable_cleanup;
	}
	if (type->isDerivedType && !transpile_arguments(file, fc, namespace, lineNum, type, &arguments)) {
		goto variable_cleanup;
	}

	process_spaces();
	if (!process_match(")")) {
		goto variable_cleanup;
	}

	process_spaces();
	if (!process_match(";")) {
		goto variable_cleanup;
	}

	const struct Variable variable = { *name, baseType->type };
	if (!add_variable(&Variables, &variable)) {
		goto variable_cleanup;
	}

	fputs("extern const struct ", fh);
	if (type->isDerivedType) {
		fprintf(fh, "%.*s_", FSTRING(namespace));
	}
	fprintf(fh, "%.*s %.*s_%.*s;\n\n", FSTRING(&typeName), FSTRING(namespace), FSTRING(name));

	fputs("const struct ", fc);
	if (type->isDerivedType) {
		fprintf(fc, "%.*s_", FSTRING(namespace));
	}
	fprintf(fc, "%.*s %.*s_%.*s = NEW_", FSTRING(&typeName), FSTRING(namespace), FSTRING(name));
	if (type->isDerivedType) {
		fputs("EXT_", fc);
	}
	fprintf(fc, "%.*s(", FSTRING(baseType->capitalBaseTypeName));
	size_t j = 0;
	for (size_t i = 0; i < baseType->fields.count && j < arguments.count; ++i, ++j) {
		if (j != 0) {
			fputs(", ", fc);
		}

		const struct Field *const field = baseType->fields.fields + i;
		switch (field->type) {
			case ArrayType:
			case BooleanType:
			case IntegerType:
			case StringType:
				break;
			case MethodType:
				if (type->isDerivedType && string_exists(&type->overridenMethodNames, &field->name)) {
					fprintf(fc, "%.*s_%.*s", FSTRING(namespace), FSTRING(&type->name));
				} else {
					fprintf(fc, "backend_default_%.*s", FSTRING(&baseType->name));
				}
				fputc('_', fc);
				break;
			case StringGeneratorType:
				if (j + 1 == arguments.count) {
					EMIT_PROG_ERROR("An unrecoverable error occurred");
					return false;
				}

				fprintf(fc, "%.*s, ", FSTRING(arguments.strings + j));
				++j;
				break;
			case EnumType:
				fprintf(fc, "%.*s_%.*s_", FSTRING(namespace), FSTRING(&field->internalTypeName));
				break;
			case StructType:
				const struct Type *const type = get_type(&field->internalTypeName);
				if (nullptr == type) {
					EMIT_PROG_ERROR("An unrecoverable error occurred");
					return false;
				}

				fprintf(fc, "USE_%.*s(%.*s_", FSTRING(type->capitalBaseTypeName), FSTRING(namespace));
				break;
		}

		fprintf(fc, "%.*s", FSTRING(arguments.strings + j));
		if (StructType == field->type) {
			fputc(')', fc);
		}
	}
	if (type->isDerivedType) {
		for (size_t i = 0; i < type->fields.count && j < arguments.count; ++i, ++j) {
			if (j != 0) {
				fputs(", ", fc);
			}

			const struct Field *const field = type->fields.fields + i;
			switch (field->type) {
				case ArrayType:
				case BooleanType:
				case IntegerType:
				case MethodType:
				case StringType:
					break;
				case StringGeneratorType:
					EMIT_PROG_ERROR("An unrecoverable error occurred");
					return false;
				case EnumType:
					fprintf(fc, "%.*s_%.*s_", FSTRING(namespace), FSTRING(&field->internalTypeName));
					break;
				case StructType:
					const struct Type *const type = get_type(&field->internalTypeName);
					if (nullptr == type) {
						EMIT_PROG_ERROR("An unrecoverable error occurred");
						return false;
					}

					fprintf(fc, "USE_%.*s(%.*s_", FSTRING(type->capitalBaseTypeName), FSTRING(namespace));
					break;
			}

			fprintf(fc, "%.*s", FSTRING(arguments.strings + j));
			if (StructType == field->type) {
				fputc(')', fc);
			}
		}
	}

	fputs(");\n\n", fc);

	status = true;

variable_cleanup:
	j = 0;
	for (size_t i = 0; i < baseType->fields.count && j < arguments.count; ++i, ++j) {
		const enum FieldType fieldType = baseType->fields.fields[i].type;
		if (ArrayType == fieldType) {
			free((void *)arguments.strings[j].str);
		} else if (StringGeneratorType == fieldType) {
			// Skip over second argument used for function pointer
			++j;
		}
	}
	if (type->isDerivedType) {
		for (size_t i = 0; i < type->fields.count && j < arguments.count; ++i, ++j) {
			const enum FieldType fieldType = type->fields.fields[i].type;
			if (ArrayType == fieldType) {
				free((void *)arguments.strings[j].str);
			} else if (StringGeneratorType == fieldType) {
				// Skip over second argument used for function pointer
				++j;
			}
		}
	}
	free(arguments.strings);

	return status;
}

// namespace NamespaceName;
// BaseType TypeName { ... }
// BaseType VariableName = TypeName(...);
[[nodiscard]] static bool transpile(
	const char8_t *restrict pFile, const char *const restrict hPath, const char *const restrict cPath
) {
	bool status = false;
	uint16_t lineNum_ = 0;
	uint16_t *const lineNum = &lineNum_;
	const char8_t *restrict *const file = &pFile;

	process_spaces();

	const char8_t *const exprStart = *file;
	if (!process_match("namespace")) {
		EMIT_LEX_ERROR();
		return false;
	}

	process_spaces();

	const char8_t *tokenStart = *file;
	if (!process_identifier()) {
		EMIT_LEX_ERROR();
		return false;
	}
	const struct String namespace = NEW_TOKEN_STRING();

	process_spaces();

	if (!process_match(";")) {
		EMIT_LEX_ERROR();
		return false;
	}


	FILE *const fh = fopen(hPath, "wb");
	if (nullptr == fh) {
		EMIT_PROG_ERROR("unable to open %s", hPath);
		return false;
	}

	FILE *const fc = fopen(cPath, "wb");
	if (nullptr == fc) {
		EMIT_PROG_ERROR("unable to open %s", cPath);
		fclose(fh);
		return false;
	}

	fprintf(fh, "\
#ifndef UTA_GEN_%.*s_H\n\
#define UTA_GEN_%.*s_H\n\
\n\
#include <stddef.h>\n\
#include <stdint.h>\n\
\n\
#include \"backend.h\"\n\
\n", FSTRING(&namespace), FSTRING(&namespace));

		fprintf(fc, "\
#include \"backend.h\"\n\
#include \"%s\"\n\n", hPath);


	for (process_spaces(); u8'\0' != **file; process_spaces()) {
		const struct Type *baseType;
		if (process_match("Action")) {
			baseType = &ActionType;
		} else if (process_match("Room")) {
			baseType = &RoomType;
		} else if (process_match("Screen")) {
			baseType = &ScreenType;
		} else if (process_match("enum")) {
			if (!transpile_enum(file, fh, &namespace, lineNum)) {
				EMIT_LEX_ERROR();
				goto cleanup;
			}

			continue;
		} else {
			EMIT_LEX_ERROR();
			goto cleanup;
		}

		process_spaces();

		tokenStart = *file;
		if (!process_identifier()) {
			return false;
		}
		const struct String name = NEW_TOKEN_STRING();

		process_spaces();
		if (process_match("{")) {
			if (transpile_type(file, fh, fc, &namespace, lineNum, baseType, &name)) {
				continue;
			}
		} else if (process_match("=")) {
			if (transpile_variable(file, fh, fc, &namespace, lineNum, baseType, &name)) {
				continue;
			}
		}

		EMIT_LEX_ERROR();
		goto cleanup;
	}

	status = true;

	fprintf(fh, "\
extern const struct Room *const %.*s_Rooms[];\n\
extern const size_t %.*s_RoomCount;\n\
\n\
#endif // UTA_GEN_%.*s_H\n",
	FSTRING(&namespace), FSTRING(&namespace), FSTRING(&namespace));

	fprintf(fc, "const struct Room *const %.*s_Rooms[] = { ", FSTRING(&namespace));
	bool printedFirst = false;
	for (size_t i = 0; i < Variables.count; ++i) {
		const struct Variable *const variable = Variables.variables + i;
		if (RoomStructType != variable->type) {
			continue;
		}

		if (printedFirst) {
			fputs(", ", fc);
		}
		printedFirst = true;

		fprintf(fc, "&%.*s_%.*s", FSTRING(&namespace), FSTRING(&variable->name));
	}
	fprintf(fc, "\
 };\n\
const size_t %.*s_RoomCount = ARR_COUNT(%.*s_Rooms);\n",
	FSTRING(&namespace), FSTRING(&namespace));

cleanup:
	fclose(fc);
	fclose(fh);
	return status;
}


#define USAGE "Usage: %s input output_header output_source extension_name\n"
#define PATH_CHECK(idx, expectedExt, error)                                   \
	path = argv[idx];                                                           \
	ext = strstr(path, expectedExt);                                            \
	if (nullptr == ext || path == ext || '\0' != ext[sizeof expectedExt - 1]) { \
		EMIT_PROG_ERROR(error);                                                   \
		fprintf(stderr, USAGE, argv[0]);                                          \
		return EXIT_FAILURE;                                                      \
	}

int main(const int argc, const char *const argv[const static argc]) {
	programName = argv[0];

	bool status = true;
	if (1 == argc || (2 == argc && 0 == strcmp(argv[1], "-h"))) {
		printf(USAGE, argv[0]);
		return 1 == argc ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	if (4 != argc) {
		fprintf(stderr, USAGE, argv[0]);
		return EXIT_FAILURE;
	}

	const char *path, *ext;
	inputPath                     = PATH_CHECK(1, ".uta", "input must be a uta source file");
	const char *const outputHPath = PATH_CHECK(2,   ".h", "output header must be a c header file");
	const char *const outputCPath = PATH_CHECK(3,   ".c", "output source must be a c source file");

	int fd = open(inputPath, O_RDONLY);
	if (-1 == fd) {
		EMIT_PROG_ERROR("unable to open %s", inputPath);
		return EXIT_FAILURE;
	}

	struct stat st;
	if (-1 == fstat(fd, &st)) {
		status = false;
		EMIT_PROG_ERROR("unable to process %s", inputPath);
	}

	void *file = MAP_FAILED;
	if (status) {
		file = mmap(nullptr, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (MAP_FAILED == file) {
			status = false;
			EMIT_PROG_ERROR("unable to read from %s", inputPath);
		}
	}

	status = status && setup_type_arrays();
	status = status && transpile(file, outputHPath, outputCPath);

	for (size_t i = 0; i < Enums.count; ++i) {
		const struct Enum *const enumType = Enums.enumTypes + i;
		free(enumType->valueNames.strings);
	}
	free(Enums.enumTypes);

	free(Variables.variables);

	for (size_t i = 0; i < Types.count; ++i) {
		const struct Type *const type = Types.types + i;
		free(type->fields.fields);
		free(type->overridenMethodNames.strings);
	}
	free(Types.types);

	munmap(file, (size_t)st.st_size);
	close(fd);

	return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
