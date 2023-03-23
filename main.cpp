#include <stdio.h>
#include "stack.h"


int main() {
    struct Stack stack = {};
    STACK_CTOR(&stack, 2);
    stack_push(&stack, 90);
    stack_push(&stack, 3);

    stack_dtor(&stack);
    return 0;
}