#ifndef LUNA_EXPORT_H
#define LUNA_EXPORT_H

#include "parse.h"

#define SECTION_PADDING_U8  (u8)0xff
#define SECTION_PADDING_U32 (u32)0xffff'ffff

string export_flat_binary(const Object* o);

#endif // LUNA_EXPORT_H
