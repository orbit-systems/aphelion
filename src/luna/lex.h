#ifndef _LEX_H
#define _LEX_H

#include "common/arena.h"
#include "common/vec.h"
#include "luna.h"

extern char* op_mnemonic[256];
int lex(Vec(LunaToken) *, Arena*, const char *);

#endif // _LEX_H
