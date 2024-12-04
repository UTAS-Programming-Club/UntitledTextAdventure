// Based on https://github.com/HaxeFoundation/hashlink/blob/9e16d8c/src/hlc_main.c
/*
 * Copyright (C)2015-2016 Haxe Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#include <stdbool.h>
#include <stddef.h>

#include "hllib.h"

#ifndef HL_MAKE
#  include <hl/hashes.c>
#  include <hl/functions.c>
#  include <hl/BaseType.c>
#  include <_std/String.c>
#  include <_std/Date.c>
#  include <hl/types/ArrayAccess.c>
#  include <hl/types/ArrayBase.c>
#  include <hl/types/ArrayBytes_Int.c>
#  include <hl/types/ArrayBytes_hl_UI16.c>
#  include <hl/types/ArrayBytes_hl_F32.c>
#  include <hl/types/ArrayBytes_Float.c>
#  include <_std/StringBuf.c>
#  include <_std/SysError.c>
#  include <backend/Room.c>
#  include <backend/GlobalData.c>
#  include <hl/types/ArrayObj.c>
#  include <haxe/Exception.c>
#  include <haxe/exceptions/PosException.c>
#  include <haxe/exceptions/NotImplementedException.c>
#  include <haxe/iterators/ArrayIterator.c>
#  include <haxe/iterators/ArrayKeyValueIterator.c>
#  include <hl/NativeArrayIterator_Dynamic.c>
#  include <hl/NativeArrayIterator_Int.c>
#  include <hl/types/BytesIterator_Float.c>
#  include <hl/types/BytesIterator_Int.c>
#  include <hl/types/BytesIterator_hl_F32.c>
#  include <hl/types/BytesIterator_hl_UI16.c>
#  include <hl/types/ArrayDynIterator.c>
#  include <hl/types/ArrayObjIterator.c>
#  include <_std/Std.c>
#  include <hl/_Bytes/Bytes_Impl_.c>
#  include <_std/Sys.c>
#  include <_std/Type.c>
#  include <frontends/TestFrontend.c>
#  include <haxe/NativeStackTrace.c>
#  include <haxe/ds/ArraySort.c>
#  include <hl/types/ArrayDyn.c>
#  include <hl/init.c>
#  include <hl/reflect.c>
#  include <hl/types.c>
#  include <hl/globals.c>
#endif

#include "../backend/GlobalData-haxe.c"

#define HL_MAKE
#define main hl_main
#include "pcgame.c"
#undef main

extern void *hl_functions_ptrs[];
extern hl_type *hl_functions_types[];
void hl_init_types(hl_module_context *ctx);

static vdynamic *call(hl_type *fun_type, void *fun, int argc, vdynamic **argv, bool *failed) {
  bool failed2;
  if (!failed) {
    failed = &failed2;
  }

  vclosure cl = { 0 };
  cl.t = fun_type;
  cl.fun = fun;

  vdynamic *ret = hl_dyn_call_safe(&cl, argv, argc, failed);
  if(*failed) {
    varray *stack = hl_exception_stack();
    uprintf(USTR("Uncaught exception: %s\n"), hl_to_string(ret));
    for (int i = 0; i < stack->size; ++i) {
      uprintf(USTR("Called from %s\n"), hl_aptr(stack, uchar *)[i]);
    }
  }

  return ret;
}

static hl_type *find_fun_type(const void *fun) {
  const void **fun_ptr = (const void **)hl_functions_ptrs;
  hl_type **fun_type = hl_functions_types;

  do {
    if (*fun_ptr == fun) {
      return *fun_type;
    }

    ++fun_ptr;
    ++fun_type;
  } while (*fun_ptr != fun);

  return NULL;
}

vdynamic *hl_lib_call(void *fun, int argc, vdynamic **argv, bool *failed) {
  hl_type *fun_type = find_fun_type(fun);
  if (!fun_type) {
    if (failed) {
      *failed = true;
    }

    return NULL;
  }

  return call(fun_type, fun, argc, argv, failed);
}

bool hl_lib_setup(int argc, char **argv) {
  vdynamic *ret;

  hl_global_init();
  hl_register_thread(&ret);
  hl_sys_init((void**)(argv + 1), argc - 1, NULL);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
  hl_setup_callbacks(hlc_static_call, hlc_get_wrapper);
#pragma GCC diagnostic pop

  hl_type_fun tf = { 0 };
  hl_type clt = { 0 };
  bool failed;

  tf.ret = &hlt_void;
  clt.kind = HFUN;
  clt.fun = &tf;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
  call(&clt, hl_entry_point, 0, NULL, &failed);
#pragma GCC diagnostic pop
  return !failed;
}

void hl_lib_cleanup(void) {
  hl_global_free();
}
