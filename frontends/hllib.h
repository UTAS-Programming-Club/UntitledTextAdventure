#ifndef HL_LIB_H
#define HL_LIB_H

#include <hl.h>

bool hl_lib_setup(int argc, char *argv[]);
void hl_lib_cleanup(void);

vdynamic *hl_lib_call(void *fun, int argc, vdynamic **argv, bool *failed);

#endif // HL_LIB_H
