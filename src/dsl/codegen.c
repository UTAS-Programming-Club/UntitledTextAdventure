#include <inttypes.h>  // for PRIu64
#include <stdio.h>     // for fprintf, fputs, fclose, size_t, FILE, fopen, fputc
#include <stdlib.h>    // for free
#include <uchar.h>     // for char8_t

#include "dsl/dsl.h"   // IWYU pragma: associated

#define FSTRING(string) (int)(string)->strLen, (string)->str
#define FTOKEN(idString) FSTRING(&idString->string) // Only use for IdentifierToken


DYN_ARRAY_IMPL(StringInfo, struct String, string)


static void write_str(FILE *const restrict f, const size_t strLen, const char8_t str[const restrict static strLen]) {
  for (size_t i = 0; i < strLen; ++i) {
    const char8_t chr = str[i];
    if (u8'\n' == chr) {
      fputs("\\", f);
    }

    fputc(chr, f);
  }
}

[[nodiscard]] bool codegen(const struct ExpressionInfo *const restrict exprs, const char *const restrict headerPath, const char *const restrict sourcePath, const char *const restrict extensionName) {
  struct StringInfo rooms = {};

  FILE *const fh = fopen(headerPath, "wb");
  if (nullptr == fh) {
    EMIT_PROG_ERROR("unable to open %s", headerPath);
    return false;
  }

  fprintf(fh, "\
#ifndef UTA_GEN_%s_H\n\
#define UTA_GEN_%s_H\n\
\n\
#include <stddef.h>\n\
\n", extensionName, extensionName);

  FILE *const fc = fopen(sourcePath, "wb");
  if (nullptr == fc) {
    EMIT_PROG_ERROR("unable to open %s", sourcePath);
    fclose(fh);
    return false;
  }

  fprintf(fc, "\
#include <stddef.h>\n\
\n\
#include \"backend.h\"\n\
#include \"coregame2.h\"\n\
#include \"%s\"\n\n", headerPath);

  for (size_t i = 0; i < exprs->count; ++i) {
    const struct Expression *const expr = exprs->exprs + i;
    switch (expr->type) {
      case TypeDeclarationExpression:
        fprintf(fh, "struct %.*s {\n  struct %.*s base;\n};\n\n",
          FSTRING(expr->typeDeclaration.idChildTypeName), FTOKEN(expr->idName)
        );
        break;
      case ActionDefinitionExpression:
        fprintf(fh, "extern const struct %.*s %s_%.*s;\n\n",
          FTOKEN(expr->action.idTypeName), extensionName, FTOKEN(expr->idName)
        );

        fprintf(fc, "const struct %.*s %s_%.*s = NEW_",
          FTOKEN(expr->action.idTypeName), extensionName, FTOKEN(expr->idName)
        );
        if (expr->action.isDerivedType) {
          fputs("EXT_", fc);
        }
        fputs("ACTION(", fc);
        write_str(fc, expr->action.strTitle->string.strLen, expr->action.strTitle->string.str);
        fprintf(fc, ", %.*s, %.*s);\n\n",
          FTOKEN(expr->action.idVisiblityCheckerFunc),
          FTOKEN(expr->action.idTriggerHandlerFunc)
        );
        break;
      case RoomDefinitionExpression:
        struct String room = { expr->idName->string.str, expr->idName->string.strLen };
        if (!add_string(&rooms, &room)) {
          free(rooms.strings);
          fclose(fc);
          fclose(fh);
          return false;
        }
        fprintf(fh, "extern const struct %.*s %s_%.*s;\n\n",
          FTOKEN(expr->room.idTypeName), extensionName, FTOKEN(expr->idName)
        );

        fprintf(fc, "const struct %.*s %s_%.*s = NEW_",
          FTOKEN(expr->room.idTypeName), extensionName, FTOKEN(expr->idName)
        );
        if (expr->room.isDerivedType) {
          fputs("EXT_", fc);
        }
        fprintf(fc, "ROOM(%" PRIu64 ", %" PRIu64 ", ",
          expr->room.intX->integer, expr->room.intY->integer
        );
        write_str(fc, expr->room.strBody->string.strLen, expr->room.strBody->string.str);
        fputs(");\n\n", fc);
        break;
      case ScreenDefinitionExpression:
        fprintf(fh, "extern const struct %.*s %s_%.*s;\n\n",
          FTOKEN(expr->screen.idTypeName), extensionName, FTOKEN(expr->idName)
        );

        fprintf(fc, "static const struct Action *const %s_%.*s_Actions[] = { ",
          extensionName, FTOKEN(expr->idName)
        );
        for (size_t i = 0; i < expr->screen.actions.count; ++i) {
          if (0 != i) {
            fputs(", ", fc);
          }
          const struct Token *const token = expr->screen.actions.tokens + i;
          fprintf(fc, "USE_ACTION(%.*s)", (int)token->string.strLen, token->string.str);
        }
        fputs(" };\n", fc);

        fprintf(fc, "const struct %.*s %s_%.*s = NEW_",
          FTOKEN(expr->screen.idTypeName), extensionName, FTOKEN(expr->idName)
        );
        if (expr->screen.isDerivedType) {
          fputs("EXT_", fc);
        }
        fputs("SCREEN(", fc);
        if (expr->screen.isBodyFunc) {
          fputs("NULL, ", fc);
        }
        write_str(fc, expr->screen.strBody->string.strLen, expr->screen.strBody->string.str);
        if (!expr->screen.isBodyFunc) {
          fputs(", backend_default_screen_body_generator", fc);
        }
        fprintf(fc, ", %s_%.*s_Actions);\n\n",
          extensionName, FTOKEN(expr->idName)
        );
        break;
    }
  }

  fprintf(fc, "const struct Room *const %s_Rooms[] = { ", extensionName);
  for (size_t i = 0; i < rooms.count; ++i) {
    const struct String *const room = rooms.strings + i;
    if (i > 0) {
      fputs(", ", fc);
    }
    fprintf(fc, "USE_ROOM(%s_%.*s)", extensionName, FSTRING(room));
  }
  fputs(" };\n", fc);
  fprintf(fc, "const size_t %s_RoomCount = ARR_COUNT(%s_Rooms);\n", extensionName, extensionName);

  fprintf(fh, "\
extern const size_t %s_RoomCount;\n\
extern const struct Room *const %s_Rooms[];\n\
\n\
#endif // UTA_GEN_%s_H\n", extensionName, extensionName, extensionName);

  free(rooms.strings);
  fclose(fc);
  fclose(fh);
  return true;
}
