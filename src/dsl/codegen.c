#include <stdio.h>    // for fprintf, fputs, fclose, size_t, FILE, fopen, fputc
#include <stdlib.h>   // for free
#include <uchar.h>    // for char8_t

#include "dsl/dsl.h"  // IWYU pragma: associated

DYN_ARRAY_DEF(StringInfo, struct String, string)
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
      case ActionDefintionExpression:
        fprintf(fh, "extern const struct %.*s %s_%.*s;\n\n",
          (int)expr->action.idTypeName->strLen, expr->action.idTypeName->str,
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str
        );
        fprintf(fc, "const struct %.*s %s_%.*s = NEW_",
          (int)expr->action.idTypeName->strLen, expr->action.idTypeName->str,
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str
        );
        if (expr->action.isDerivedType) {
          fputs("EXT_", fc);
        }
        fputs("ACTION(", fc);
        write_str(fc, expr->action.strTitle->string.strLen, expr->action.strTitle->string.str);
        fprintf(fc, ", %.*s, %.*s);\n\n",
          (int)expr->action.idVisiblityCheckerFunc->string.strLen, expr->action.idVisiblityCheckerFunc->string.str,
          (int)expr->action.idTriggerHandlerFunc->string.strLen, expr->action.idTriggerHandlerFunc->string.str
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
        fprintf(fh, "extern const struct Room %s_%.*s;\n\n",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str
        );
        fprintf(fc, "const struct Room %s_%.*s = NEW_ROOM(%w64u, %w64u, ",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str,
          expr->room.intX->integer, expr->room.intY->integer,
          (int)expr->room.strBody->string.strLen, expr->room.strBody->string.str
        );
        write_str(fc, expr->room.strBody->string.strLen, expr->room.strBody->string.str);
        fputs(");\n\n", fc);
        break;
      case ScreenDefinitionExpression:
        fprintf(fh, "extern const struct Screen %s_%.*s;\n\n",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str
        );
        fprintf(fc, "const struct Screen %s_%.*s = NEW_",
          extensionName,
          (int)expr->idName->string.strLen, expr->idName->string.str,
          (int)expr->screen.strBody->string.strLen, expr->screen.strBody->string.str
        );
        if (expr->screen.isBodyFunc) {
          fputs("VAR_", fc);
        }
        fputs("SCREEN(", fc);
        write_str(fc, expr->screen.strBody->string.strLen, expr->screen.strBody->string.str);
        for (size_t i = 0; i < expr->screen.actions.count; ++i) {
          const struct Token *const token = expr->screen.actions.tokens + i;
          fprintf(fc, ", USE_ACTION(%.*s)", (int)token->string.strLen, token->string.str);
        }
        fputs(");\n\n", fc);
        break;
    }
  }

  fprintf(fc, "const struct Room *const %s_Rooms[] = { ", extensionName);
  for (size_t i = 0; i < rooms.count; ++i) {
    const struct String *const room = rooms.strings + i;
    if (i > 0) {
      fputs(", ", fc);
    }
    fprintf(fc, "&%s_%.*s", extensionName, (int)room->strLen, room->str);
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
