#include "common/type.h"
#include "common/arena.h"

#include <stdio.h>

int main() {
	Arena a;
    arena_init(&a);
    puts("hello");
    return 0;
}
