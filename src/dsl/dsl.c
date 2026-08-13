#include <ctype.h>     // for isalnum, isdigit, isspace
#include <fcntl.h>     // for O_RDONLY, open
#include <inttypes.h>  // for uint16_t, PRIu16, uint_fast8_t
#include <stdio.h>     // for size_t, fprintf, stderr, fputs, FILE, fclose, fopen, snprintf, printf
#include <stdlib.h>    // for free, EXIT_FAILURE, realloc, EXIT_SUCCESS, malloc
#include <string.h>    // for memcpy, strstr, strncmp, strcmp
#include <sys/mman.h>  // for MAP_FAILED, MAP_PRIVATE, PROT_READ, mmap, munmap
#include <sys/stat.h>  // for stat, fstat
#include <uchar.h>     // for char8_t
#include <unistd.h>    // for close

const char *programName;
const char *inputPath;

#define EMIT_PROG_ERROR(error, ...) \
  fprintf(stderr, "%s: \u001b[0;31merror\u001b[0m: " error "\n", programName __VA_OPT__(,) __VA_ARGS__)

#define DYN_ARRAY_DEF(typeName, baseTypeName, varName) struct typeName {                                            \
  baseTypeName *varName ## s;                                                                                       \
  size_t count;                                                                                                     \
  size_t length;                                                                                                    \
};                                                                                                                  \
                                                                                                                    \
[[nodiscard]] static bool add_ ## varName(struct typeName *const varName ## s, const baseTypeName *const varName) { \
if (varName ## s->count + 1 >= varName ## s->length) {                                                              \
    size_t newLen = 2 * varName ## s->count;                                                                        \
    if (0 == newLen) {                                                                                              \
      newLen = 8;                                                                                                   \
    }                                                                                                               \
                                                                                                                    \
    baseTypeName *newArr = realloc(varName ## s->varName ## s, newLen * sizeof *varName ## s->varName ## s);        \
    if (nullptr == newArr) {                                                                                        \
      EMIT_PROG_ERROR("An unrecoverable error occurred");                                                           \
      return false;                                                                                                 \
    }                                                                                                               \
                                                                                                                    \
    varName ## s->length = newLen;                                                                                  \
    varName ## s->varName ## s = newArr;                                                                            \
  }                                                                                                                 \
                                                                                                                    \
  memcpy(varName ## s->varName ## s + varName ## s->count, varName, sizeof *varName);                               \
  ++varName ## s->count;                                                                                            \
  return true;                                                                                                      \
}

// TODO: Restore full error line reporting, also make sure unicode prints then (doesn't now unless only 1 byte)
#define EMIT_LEX_ERROR() EMIT_PROG_ERROR("%s:%" PRIu16 ":%td: Unexpected character: %c", inputPath, *lineNum + 1, *file - exprStart + 1, **file)


#define NEW_STATIC_STRING(str) (const struct String){ u8 ## str, sizeof u8 ## str - 1 }
#define NEW_TOKEN_STRING() (const struct String){ tokenStart, (size_t)(*file - tokenStart) }
struct String {
  const char8_t *str;
  size_t strLen;
};
DYN_ARRAY_DEF(StringArray, struct String, string)

// TODO: Merge BooleanType & IntegerType (value types), and ActionType & RoomType & ScreenType (dsl types)?
enum CType {
  ArrayCType,   // ItemType[] where ItemType is any CType other than ArrayType,      only used for fields/function parameters
  BooleanCType, // bool in both dsl and C,                                           only used for function return types
  EnumCType,    // enum EnumName in both dsl and C,                                  only used for fields/function parameters
  IntegerCType, // One of IntegerTypes in both dsl and C,                            only used for fields/function parameters
  MethodCType,  // ReturnType Name() where ReturnType is BooleanType or IntegerType, only used for fields/function parameters
  StringCType,  // string in dsl and const char * in C,                              only used for fields/function parameters

  ActionCType,  // Only used for fields/function parameters
  RoomCType,    // Only used for fields/function parameters
  ScreenCType   // Only used for fields/function parameters
};

struct Enum {
  struct String name;
  struct StringArray valueNames;
};
DYN_ARRAY_DEF(EnumArray, struct Enum, enumType)

struct Field {
  enum CType type;
  struct String name; // Only set if parent Type.isDerivedType or type is MethodType (functionName)

  enum CType arrayBaseType; // Only set if type is ArrayCType
  struct String internalTypeName; // integerTypeName if type or arrayBaseType is IntegerCType, returnTypeName if MethodCType, only set if parent Type.isDerivedType
};
DYN_ARRAY_DEF(FieldArray, struct Field, field)

struct Type {
  enum CType type; // Only allowed to be ActionCType, RoomCType & ScreenCType
  struct String name;
  bool isDerivedType;
  struct FieldArray fields;
};
DYN_ARRAY_DEF(TypeArray, struct Type, type)

static struct Type ActionType = { ActionCType, NEW_STATIC_STRING("Action") };
static struct Field ActionTitle = { StringCType };
static struct Field ActionVisibilityChecker = {
  MethodCType, NEW_STATIC_STRING("backend_default_action_visibility_checker"), BooleanCType
};
static struct Field ActionTriggerHandler = {
  MethodCType, NEW_STATIC_STRING("backend_default_action_trigger_handler"), BooleanCType
};
static struct TypeArray ActionTypes = {};

static struct Type RoomType = { RoomCType, NEW_STATIC_STRING("Room") };
static struct Field RoomX = { IntegerCType };
static struct Field RoomY = { IntegerCType };
static struct Field RoomBody = { StringCType };
static struct TypeArray RoomTypes = {};

static struct Type ScreenType = { ScreenCType, NEW_STATIC_STRING("Screen") };
// TODO: Support body generator method
static struct Field ScreenBody = { StringCType };
static struct Field ScreenActions = { ArrayCType, .arrayBaseType = ActionCType };
static struct TypeArray ScreenTypes = {};

static const struct String IntegerTypes[] = {
  NEW_STATIC_STRING("int8_t"),  NEW_STATIC_STRING("int16_t"),  NEW_STATIC_STRING("int32_t"),
  NEW_STATIC_STRING("uint8_t"), NEW_STATIC_STRING("uint16_t"), NEW_STATIC_STRING("uint32_t")
};
static const size_t IntegerTypeCount = sizeof IntegerTypes / sizeof *IntegerTypes;

[[nodiscard]] static bool setup_type_arrays() {
 return add_field(&ActionType.fields, &ActionTitle) &&
        add_field(&ActionType.fields, &ActionVisibilityChecker) &&
        add_field(&ActionType.fields, &ActionTriggerHandler) &&
        add_type(&ActionTypes, &ActionType) &&

        add_field(&RoomType.fields, &RoomX) && add_field(&RoomType.fields, &RoomY) &&
        add_field(&RoomType.fields, &RoomBody) && add_type(&RoomTypes, &RoomType) &&

        add_field(&ScreenType.fields, &ScreenBody) && add_field(&ScreenType.fields, &ScreenActions) &&
        add_type(&ScreenTypes, &ScreenType);
}

static void free_type_array(const struct TypeArray *const array) {
  for (size_t i = 0; i < array->count; ++i) {
    free(array->types[i].fields.fields);
  }

  free(array->types);
}

[[nodiscard]] static bool string_equals(const struct String *const restrict str1, const struct String *const restrict str2) {
  if (str1->strLen != str2->strLen) {
    return false;
  }

  return 0 == strncmp((const char *)str1->str, (const char *)str2->str, str1->strLen);
}

[[nodiscard]] static const struct Type *get_type(const struct TypeArray *const restrict types, const struct String *const restrict typeName) {
  for (size_t i = 0; i < types->count; ++i) {
    const struct Type *type = types->types + i;
    if (string_equals(&type->name, typeName)) {
      return type;
    }
  }

  return nullptr;
}

[[nodiscard]] static bool type_is_integer(const struct String *type) {
  for (size_t i = 0; i < IntegerTypeCount; ++i) {
    if (string_equals(IntegerTypes + i, type)) {
      return true;
    }
  }

  return false;
}


static struct StringArray ActionVariableNames = {};
static struct StringArray RoomVariableNames = {};
static struct StringArray ScreenVariableNames = {};

[[nodiscard]] static struct StringArray *get_variable_names(const enum CType variableType) {
  switch (variableType) {
    case ActionCType:
      return &ActionVariableNames;
      break;
    case RoomCType:
      return &RoomVariableNames;
      break;
    case ScreenCType:
      return &ScreenVariableNames;
      break;
    default:
      return nullptr;
  }
}

[[nodiscard]] static bool add_variable(const enum CType variableType, const struct String *const variableName) {
  struct StringArray *const variableNames = get_variable_names(variableType);
  if (nullptr == variableNames) {
    return false;
  }

  return add_string(variableNames, variableName);
}

[[nodiscard]] static bool variable_exists(const enum CType variableType, const struct String *const restrict variableName) {
  struct StringArray *const variableNames = get_variable_names(variableType);
  if (nullptr == variableNames) {
    return false;
  }

  for (size_t i = 0; i < variableNames->count; ++i) {
    const struct String *storedVariableName = variableNames->strings + i;
    if (string_equals(storedVariableName, variableName)) {
      return true;
    }
  }

  return false;
}


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

[[nodiscard]] static bool enum_value_exists(const struct String *const restrict enumName, const struct String *const restrict enumValueName) {
  const struct Enum *const enumType = get_enum(enumName);
  if (nullptr == enumType) {
    return false;
  }

  const struct StringArray *const enumValueNames = &enumType->valueNames;
  for (size_t i = 0; i < enumValueNames->count; ++i) {
    if (string_equals(enumValueNames->strings + i, enumValueName)) {
      return true;
    }
  }

  return false;
}


// TODO: Ensure this supports unicode
#define process_match(match) process_match(file, sizeof u8 ## match - 1, u8 ## match)
[[nodiscard]] static bool (process_match)(const char8_t *restrict *const restrict file, size_t strLen, const char8_t match[const restrict static strLen]) {
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
[[nodiscard]] static bool (process_argument)(const char8_t *restrict *const restrict file, const struct Field *const restrict expectedType, struct String *const restrict argument);

#define process_array(itemType, arrayItems) process_array(file, lineNum, itemType, &arrayItems)
[[nodiscard]] static bool (process_array)(const char8_t *restrict *const restrict file, uint16_t *const restrict lineNum,
                                          const struct Field *const restrict itemType, struct StringArray *const restrict arrayItems) {
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

[[nodiscard]] static bool (process_argument)(const char8_t *restrict *const restrict file, const struct Field *const restrict expectedType, struct String *const restrict argument) {
  const char8_t *const tokenStart = *file;
  const enum CType type = ArrayCType == expectedType->type ? expectedType->arrayBaseType : expectedType->type;

  bool isVariableType = false;
  switch (type) {
    case ArrayCType:
    case BooleanCType:
    case MethodCType:
      return false;
    case IntegerCType:
      if (!process_integer()) {
        return false;
      }
      break;
    // TODO: Support multi line strings
    case StringCType:
      if (!process_string()) {
        return false;
      }
      break;
    case ActionCType:
    case RoomCType:
    case ScreenCType:
      isVariableType = true;
      [[fallthrough]];
    case EnumCType:
      if (!process_identifier()) {
        return false;
      }
      break;
  }

  *argument = NEW_TOKEN_STRING();

  if (EnumCType == expectedType->type) {
    if (!enum_value_exists(&expectedType->internalTypeName, argument)) {
      return false;
    }
  } else if (isVariableType) {
    if (!variable_exists(type, argument)) {
      return false;
    }
  }

  return true;
}


#define FSTRING(string) (int)(string)->strLen, (string)->str

/* enum EnumType {
 *   EnumValue,
 *   ...
 * }
 */
[[nodiscard]] static bool transpile_enum(const char8_t *restrict *const restrict file, FILE *const restrict fh, uint16_t *const restrict lineNum) {
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

  // TODO: Add prefix to enum values
  fprintf(fh, "enum %.*s {\n", FSTRING(&name));
  for (size_t i = 0; i < valueNames.count; ++i) {
    fprintf(fh, "  %.*s,\n", FSTRING(valueNames.strings + i));
  }
  fputs("};\n\n", fh);

  const struct Enum enumType = { name, valueNames };
  return add_enumType(&Enums, &enumType);
}

// BaseType = Action | Room | Screen
/* BaseType Type {
 *   FieldType FieldName;
 *   ...
 * }
 */
[[nodiscard]] static bool transpile_type(const char8_t *restrict *const restrict file, FILE *const restrict fh,
                                         uint16_t *const restrict lineNum, struct TypeArray *const restrict types,
                                         const struct Type *const restrict baseType, const struct String *const restrict name) {
  if (nullptr != get_type(&ActionTypes, name) || nullptr != get_type(&RoomTypes, name) || nullptr != get_type(&ScreenTypes, name)) {
    // TODO: Add error
    return false;
  }

  struct FieldArray fields = {};
  for (process_spaces(); u8'\0' != **file; process_spaces()) {
    enum CType type;
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
    // TODO: Allow method fields
    // TODO: Allow string fields
    // TODO: Allow array fields of bools, integers and strings
    if (isEnum) {
      type = EnumCType;
    } else if (type_is_integer(&fieldTypeName)) {
      type = IntegerCType;
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

    struct Field field = { type, fieldName, .internalTypeName = fieldTypeName };
    if (!add_field(&fields, &field)) {
      goto type_failure;
    }
  }

  process_spaces();
  if (!process_match("}")) {
    goto type_failure;
  }

  struct Type type = { baseType->type, *name, true, fields };
  if (!add_type(types, &type)) {
    goto type_failure;
  }

  // TODO: Add prefix to type name
  fprintf(fh, "struct %.*s {\n"
              "  const struct %.*s base;\n\n",
          FSTRING(name),
          FSTRING(&baseType->name)
  );

  for (size_t i = 0; i < fields.count; ++i) {
    const struct Field *const field = fields.fields + i;

    fputs("  ", fh);

    switch (field->type) {
      case ActionCType:
      case ArrayCType:
      case BooleanCType:
      case MethodCType:
      case RoomCType:
      case ScreenCType:
        goto type_failure;
      case EnumCType:
        fputs("enum ", fh);
        [[fallthrough]];
      case IntegerCType:
        fprintf(fh, "%.*s %.*s;\n", FSTRING(&field->internalTypeName), FSTRING(&field->name));
        break;
      case StringCType:
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

[[nodiscard]] static bool transpile_parameters(const char8_t *restrict *const restrict file, FILE *const restrict fc,
                                               uint16_t *const restrict lineNum, const struct Type *const restrict type,
                                               struct StringArray *const restrict arguments) {
  for (size_t i = 0; i < type->fields.count; ++i) {
    const struct Field *field = type->fields.fields + i;
    if (MethodCType == field->type) {
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
      case ArrayCType:
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
        argument.strLen = (size_t)snprintf((char *)argument.str, argument.strLen + 1, "array%" PRIu16 "_%zu", *lineNum + 1, i);
        if (0 > argument.strLen) {
          free((void *)argument.str);
          free(arrayItems.strings);
          return false;
        }

        fputs("const ", fc);
        const char8_t *capitalName;
        switch (field->arrayBaseType) {
          case ArrayCType:
          case BooleanCType:
          case EnumCType:
          case IntegerCType:
          case MethodCType:
          case StringCType:
            free((void *)argument.str);
            free(arrayItems.strings);
            return false;
          case ActionCType:
            fputs("Action", fc);
            capitalName = u8"ACTION";
            break;
          case RoomCType:
            fputs("Room", fc);
            capitalName = u8"ROOM";
            break;
          case ScreenCType:
            fputs("Screen", fc);
            capitalName = u8"SCREEN";
            break;
        }
        fprintf(fc, " %.*s[] = { ",  FSTRING(&argument));
        for (size_t j = 0; j < arrayItems.count; ++j) {
          if (0 != j) {
            fputs(", ", fc);
          }

          fputs("USE_", fc);
          switch (field->arrayBaseType) {
            case ArrayCType:
            case BooleanCType:
            case EnumCType:
            case IntegerCType:
            case MethodCType:
            case StringCType:
              free((void *)argument.str);
              free(arrayItems.strings);
              return false;
            case ActionCType:
            case RoomCType:
            case ScreenCType:
              fprintf(fc, "%s", capitalName);
              break;
          }
          fprintf(fc, "(%.*s)", FSTRING(arrayItems.strings + j));
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
  }

  return true;
}

// BaseType = Action | Room | Screen
// BaseType VariableName = Type([... [, ... [...]]]);
[[nodiscard]] static bool transpile_variable(const char8_t *restrict *const restrict file, FILE *const restrict fh, FILE *const restrict fc,
                                             uint16_t *const restrict lineNum, const struct TypeArray *const restrict types,
                                             const char8_t *const restrict capitalName, const struct Type *const restrict baseType,
                                             const struct String *const restrict name) {
  process_spaces();

  const char8_t *tokenStart = *file;
  if (!process_identifier()) {
    return false;
  }
  const struct String typeName = NEW_TOKEN_STRING();
  const struct Type *type = get_type(types, &typeName);
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
  if (!transpile_parameters(file, fc, lineNum, baseType, &arguments)) {
    goto variable_cleanup;
  }
  if (type->isDerivedType && !transpile_parameters(file, fc, lineNum, type, &arguments)) {
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

  if (!add_variable(baseType->type, name)) {
  }

  fprintf(fh, "extern const struct %.*s %.*s;\n\n", FSTRING(&typeName), FSTRING(name));

  fprintf(fc, "const struct %.*s %.*s = NEW_", FSTRING(&typeName), FSTRING(name));
  if (type->isDerivedType) {
    fputs("EXT_", fc);
  }
  fprintf(fc, "%s(", capitalName);
  for (size_t i = 0; i < arguments.count; ++i) {
    if (i != 0) {
      fputs(", ", fc);
    }

    fprintf(fc, "%.*s", FSTRING(arguments.strings + i));
  }
  fputs(");\n\n", fc);

  status = true;

variable_cleanup:
  for (size_t i = 0; i < type->fields.count; ++i) {
    if (ArrayCType == type->fields.fields[i].type && arguments.count > i) {
      free((void *)arguments.strings[i].str);
    }
  }
  free(arguments.strings);

  return status;
}

// BaseType Type { ... }
// BaseType VariableName = Type(...);
[[nodiscard]] static bool transpile(const char8_t *restrict pFile, const char *const restrict hPath, const char *const restrict cPath, const char *const restrict extensionName) {
  bool status = false;
  uint16_t lineNum_ = 0;
  uint16_t *const lineNum = &lineNum_;
  const char8_t *restrict *const file = &pFile;

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
#ifndef UTA_GEN_%s_H\n\
#define UTA_GEN_%s_H\n\
\n\
#include <stdint.h>\n\
\n", extensionName, extensionName);

    fprintf(fc, "\
#include \"backend.h\"\n\
#include \"%s\"\n\n", hPath);


  for (process_spaces(); u8'\0' != **file; process_spaces()) {
    const char8_t *const exprStart = *file;
    const char8_t *tokenStart = *file;

    const struct Type *baseType;
    struct TypeArray *types;
    const char8_t *capitalName;
    if (process_match("Action")) {
      baseType = &ActionType;
      types = &ActionTypes;
      capitalName = u8"ACTION";
    } else if (process_match("Room")) {
      baseType = &RoomType;
      types = &RoomTypes;
      capitalName = u8"ROOM";
    } else if (process_match("Screen")) {
      baseType = &ScreenType;
      types = &ScreenTypes;
      capitalName = u8"SCREEN";
    } else if (process_match("enum")) {
      if (!transpile_enum(file, fh, lineNum)) {
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
      if (transpile_type(file, fh, lineNum, types, baseType, &name)) {
        continue;
      }
    } else if (process_match("=")) {
      if (transpile_variable(file, fh, fc, lineNum, types, capitalName, baseType, &name)) {
        continue;
      }
    }

    EMIT_LEX_ERROR();
    goto cleanup;
  }

  status = true;

  fprintf(fh, "#endif // UTA_GEN_%s_H\n", extensionName);

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

  if (5 != argc) {
    fprintf(stderr, USAGE, argv[0]);
    return EXIT_FAILURE;
  }

  const char *path, *ext;
  inputPath                     = PATH_CHECK(1, ".uta", "input must be a uta source file");
  const char *const outputHPath = PATH_CHECK(2,   ".h", "output header must be a c header file");
  const char *const outputCPath = PATH_CHECK(3,   ".c", "output source must be a c source file");
  const char *const extensionName = argv[4];

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
  status = status && transpile(file, outputHPath, outputCPath, extensionName);

  free(ActionVariableNames.strings);
  free(RoomVariableNames.strings);
  free(ScreenVariableNames.strings);

  free_type_array(&ActionTypes);
  free_type_array(&RoomTypes);
  free_type_array(&ScreenTypes);

  munmap(file, (size_t)st.st_size);
  close(fd);

  return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
