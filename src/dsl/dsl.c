#include <ctype.h>     // for isalnum, isdigit, isspace
#include <fcntl.h>     // for O_RDONLY, open
#include <stdint.h>    // for uint16_t
#include <stdio.h>     // for fprintf, size_t, stderr, FILE, fclose, fopen, fputs, printf
#include <stdlib.h>    // for EXIT_FAILURE, free, realloc, EXIT_SUCCESS
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
// TODO: Fix error with PRIu16
#define EMIT_LEX_ERROR() EMIT_PROG_ERROR("%s:%hu:%td: Unexpected character: %c", inputPath, *lineNum + 1, *file - exprStart + 1, **file)


#define NEW_STATIC_STRING(str) { str, sizeof str - 1 }
#define NEW_TOKEN_STRING() { tokenStart, (size_t)(*file - tokenStart) }
struct String {
  const char8_t *str;
  size_t strLen;
};
DYN_ARRAY_DEF(StringArray, struct String, string)

enum FieldType {
  IntegerField,
  StringField
};

struct Field {
  enum FieldType type;
  struct String typeName;
  struct String fieldName;
};
DYN_ARRAY_DEF(FieldArray, struct Field, field)

struct Type {
  struct String name;
  bool isDerivedType;
  struct FieldArray fields;
};
DYN_ARRAY_DEF(TypeArray, struct Type, type)

static struct Type ActionType = { NEW_STATIC_STRING(u8"Action") };
static struct Field ActionField1 = { StringField };
static struct TypeArray ActionTypes = {};

static struct Type RoomType = { NEW_STATIC_STRING(u8"Room") };
static struct Field RoomField1 = { IntegerField };
static struct Field RoomField2 = { IntegerField };
static struct Field RoomField3 = { StringField };
static struct TypeArray RoomTypes = {};

static struct Type ScreenType = { NEW_STATIC_STRING(u8"Screen") };
static struct Field ScreenField1 = { StringField };
static struct TypeArray ScreenTypes = {};

static const struct String IntegerTypes[] = {
  NEW_STATIC_STRING(u8"int8_t"),  NEW_STATIC_STRING(u8"int16_t"),  NEW_STATIC_STRING(u8"int32_t"),
  NEW_STATIC_STRING(u8"uint8_t"), NEW_STATIC_STRING(u8"uint16_t"), NEW_STATIC_STRING(u8"uint32_t")
};
static const size_t IntegerTypeCount = sizeof IntegerTypes / sizeof *IntegerTypes;

[[nodiscard]] static bool setup_type_arrays() {
 return add_field(&ActionType.fields, &ActionField1) && add_type(&ActionTypes, &ActionType) &&
        add_field(&RoomType.fields, &RoomField1) && add_field(&RoomType.fields, &RoomField2) &&
        add_field(&RoomType.fields, &RoomField3) && add_type(&RoomTypes, &RoomType) &&
        add_field(&ScreenType.fields, &ScreenField1) && add_type(&ScreenTypes, &ScreenType);
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


// TODO: Ensure this supports unicode
#define process_match(match) process_match(file, sizeof match - 1, match)
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
  for (; u8'\0' != **file; ++*file) {
    if (u8'\n' == **file) {
      ++*lineNum;
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

#define process_string() process_string(file)
[[nodiscard]] static bool (process_string)(const char8_t *restrict *const restrict file) {
  if (!process_match(u8"\"")) {
    return false;
  }

  for (; u8'\0' != **file; ++*file) {
    if (u8'"' == **file) {
      break;
    }
  }

  if (!process_match(u8"\"")) {
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

#define FSTRING(string) (int)(string)->strLen, (string)->str


// BaseType = Action | Room | Screen
/* BaseType name {
     FieldTypeName FieldName;
     ...
 * }
 */
[[nodiscard]] static bool transpile_type(const char8_t *restrict *const restrict file, FILE *const restrict fh,
                                         uint16_t *const restrict lineNum, struct TypeArray *const restrict types,
                                         const struct String *const restrict baseTypeName, const struct String *const restrict name) {
  if (nullptr != get_type(&ActionTypes, name) || nullptr != get_type(&RoomTypes, name) || nullptr != get_type(&ScreenTypes, name)) {
    // TODO: Add error
    return false;
  }

  struct FieldArray fields = {};
  for (process_spaces(); u8'\0' != **file; process_spaces()) {
    const char8_t *tokenStart = *file;
    if (!process_identifier()) {
      break;
    }
    const struct String fieldTypeName = NEW_TOKEN_STRING();

    if (!type_is_integer(&fieldTypeName)) {
      return false;
    }

    process_spaces();

    tokenStart = *file;
    if (!process_identifier()) {
      return false;
    }
    const struct String fieldName = NEW_TOKEN_STRING();

    process_spaces();
    if (!process_match(u8";")) {
      return false;
    }

    struct Field field = { IntegerField, fieldTypeName, fieldName };
    if (!add_field(&fields, &field)) {
      return false;
    }
  }

  process_spaces();
  if (!process_match(u8"}")) {
    return false;
  }

  struct Type type = { *name, true, fields };
  if (!add_type(types, &type)) {
    return false;
  }

  fprintf(fh, "struct %.*s {\n"
              "  const struct %.*s base;\n\n",
          FSTRING(name),
          FSTRING(baseTypeName)
  );

  for (size_t i = 0; i < fields.count; ++i) {
    const struct Field *const field = fields.fields + i;
    fprintf(fh, "  %.*s %.*s;\n",
            FSTRING(&field->typeName),
            FSTRING(&field->fieldName)
           );
  }

  fputs("};\n\n", fh);
  // TODO: Add typedef?

  return true;
}

[[nodiscard]] static bool transpile_parameters(const char8_t *restrict *const restrict file, uint16_t *const restrict lineNum,
                                               const struct Type *const restrict type, struct StringArray *const restrict arguments) {
  for (size_t i = 0; i < type->fields.count; ++i) {
    const struct Field *field = type->fields.fields + i;

    process_spaces();

    if (type->isDerivedType || i != 0) {
      if (!process_match(u8",")) {
        return false;
      }
      process_spaces();
    }

    const char8_t *tokenStart = *file;
    switch (field->type) {
      case IntegerField:
        if (!process_integer()) {
          return false;
        }
        break;
      case StringField:
        if (!process_string()) {
          return false;
        }
        break;
    }

    const struct String string = NEW_TOKEN_STRING();
    if (!add_string(arguments, &string)) {
      return false;
    }
  }

  return true;
}

// BaseType = Action | Room | Screen
// BaseType name = Type([... [, ...[...]]]);
[[nodiscard]] static bool transpile_variable(const char8_t *restrict *const restrict file, FILE *const restrict fh, FILE *const restrict fc,
                                             uint16_t *const restrict lineNum, const struct TypeArray *const restrict types,
                                             const char8_t *const restrict capital_name, const struct Type *const restrict baseType,
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
   if (!process_match(u8"(")) {
     return false;
   }

  struct StringArray arguments = {};
  if (!transpile_parameters(file, lineNum, baseType, &arguments)) {
    return false;
  }
  if (type->isDerivedType && !transpile_parameters(file, lineNum, type, &arguments)) {
    return false;
  }

  process_spaces();
  if (!process_match(u8")")) {
    return false;
  }

  process_spaces();
  if (!process_match(u8";")) {
    return false;
  }

  fprintf(fh, "extern const struct %.*s %.*s;\n\n", FSTRING(&baseType->name), FSTRING(name));

  fprintf(fc, "const struct %.*s %.*s = NEW_", FSTRING(&baseType->name), FSTRING(name));
  if (type->isDerivedType) {
    fputs("EXT_", fc);
  }
  fprintf(fc, "%s(", capital_name);
  for (size_t i = 0; i < arguments.count; ++i) {
    if (i != 0) {
      fputs(", ", fc);
    }

    fprintf(fc, "%.*s", FSTRING(arguments.strings + i));
  }
  fputs(");\n\n", fc);

  return true;
}

// TODO: Restore single- and multi-line comments
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
    const char8_t *capital_name;
    if (process_match(u8"Action")) {
      baseType = &ActionType;
      types = &ActionTypes;
      capital_name = u8"ACTION";
    } else if (process_match(u8"Room")) {
      baseType = &RoomType;
      types = &RoomTypes;
      capital_name = u8"ROOM";
    } else if (process_match(u8"Screen")) {
      baseType = &ScreenType;
      types = &ScreenTypes;
      capital_name = u8"SCREEN";
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
    if (process_match(u8"{")) {
      if (transpile_type(file, fh, lineNum, types, &baseType->name, &name)) {
        continue;
      }
    } else if (process_match(u8"=")) {
      if (transpile_variable(file, fh, fc, lineNum, types, capital_name, baseType, &name)) {
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

  if (nullptr != ActionTypes.types) {
    free(ActionTypes.types);
  }
  if (nullptr != RoomTypes.types) {
    free(RoomTypes.types);
  }
  if (nullptr != ScreenTypes.types) {
    free(ScreenTypes.types);
  }

  munmap(file, (size_t)st.st_size);
  close(fd);

  return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
