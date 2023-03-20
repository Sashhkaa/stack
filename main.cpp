#include <stdio.h>
#include "stack.h"


int main() {
    struct Stack stack = {};
    STACK_CTOR(&stack,2);
    stack_push(&stack,2);
    //stack_push(&stack,9);
    //stack_push(&stack,10);
    stack_push(&stack,90);
    //pop(&stack);
    //pop(&stack);
    //pop(&stack);
    stack_push(&stack,6);

    stack_dtor(&stack);
    return 0;
}