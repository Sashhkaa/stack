
#include <stdlib.h>

#include "stack.h"

#define STACK_OK(stk)                                     \
    do {                                                   \
        int err = stack_check(stk);                         \
        if (1) {                                             \
            FILE* err_file = fopen("err.txt", "a");           \
            STACK_DUMP(stk, err_file);                         \
            fclose(err_file);                                   \
            /*abort();*/                                         \
        }                                                         \
    } while (0)

Elem_t stack_resize(struct Stack* stack,int mode);
void canary_arragement(struct Stack* stack, void* new_data);
void stack_dtor(struct Stack* stack);
void stack_dump(struct Stack* stack, FILE* file);
int stack_is_destroyed(struct Stack* stack);
int stack_check(struct Stack* stack);
void canary_arragement(struct Stack* stack, Elem_t* new_data);

void stack_print(struct Stack* stack) {
    STACK_OK(stack);
    for (int i = 0; i < stack->size; i++)
        printf("stack->data[] = %u\n", stack->data[i]);

}

int stack_check(struct Stack* stack) { 
    if (stack == nullptr) {
        return STK_NULLPTR;
    }
    if (stack_is_destroyed(stack)) {  
        return STACK_DESTROYED;
    } 
    if (stack->data == nullptr) {
        return DATA_NULLPTR;
    }
    if (stack->capacity < 0) {
        return NEGATIV_CAPACITY;
    }
    if (stack->size < 0) {
        return NEGATIVE_SIZE;
    }
    // if (stack->size > stack->capacity) {
    //     return SIZE_OVERFLOW;
    // }
    if (stack->capacity == 0) {
        return CAPACITY_NULL;
    }
    
    if (stack->canary_left  != CANARY) {
        return STACK_UNDERFLOW;
    }

    if (stack->canary_right != CANARY) {
        return STACK_OVERFLOW;
    }

    // if (*((Canary_t*)stack->data- 1) != CANARY) {
    //     return DATA_CANARY_LEFT_DIED;
    // }
    
    // if (*((Canary_t*)(stack->data + stack->capacity)) != CANARY) {
    //     return DATA_CANARY_RIGHT_DIED;
    // }
    
    // for (int i = 0; i < stack->size; i++) { 
    //    if (stack->data[i] == POISON) {
    //        return DATA_IS_POISON;
    //     }
    // }

    // for (int i = stack->size; i < stack->capacity; i++) { 
    //     if (stack->data[i] != POISON) {
    //         return PADDING_IS_NO_POISON;
    //     }
    // }

    return NO_ERROR;
}

void stack_data_poison_filling(struct Stack* stack) {
    for (int i = stack->size; i < stack->capacity; i++) {
        stack->data[i] = POISON;
    }
}

void stack_push (struct Stack* stack, Elem_t value) { 
    STACK_OK(stack);

    if (stack->size >= stack->capacity) {
        stack_resize(stack, INCREASE_CAPACITY);
    }
    if (stack->size < stack->capacity) {
        stack_resize(stack, DECREASE_CAPACITY);
    }

    stack->data[stack->size + 1] = value;
    stack->size += 1;
    STACK_OK(stack->size);
}

Elem_t stack_resize(struct Stack* stack,int mode) { 

    if (mode == INCREASE_CAPACITY) {
        Elem_t* new_data = stack->data;
        Elem_t new_capacity = stack->capacity * 2;

        new_data = (Elem_t*)realloc((Canary_t*)new_data - 1, new_capacity * sizeof(Elem_t) + 2 * sizeof(Canary_t));
        if (new_data == nullptr) {
            new_capacity = stack->capacity + 1;
            new_data = (Elem_t*)realloc((Canary_t*)new_data - 1, new_capacity * sizeof(Elem_t) + 2 * sizeof(Canary_t)); 
        }

        if (new_data == nullptr) {
            return OUT_OF_MEMORY_FOR_RECALLOC;
        }
        else {
            stack->capacity = new_capacity; 
            canary_arragement(stack,new_data);
        }
    }

    // if (mode == DECREASE_CAPACITY) {
    //     Elem_t new_capacity = stack->capacity / 2;
    //     Elem_t* new_data = (Elem_t*)malloc(new_capacity * sizeof(Elem_t) + 2 * sizeof(Canary_t));
    //     stack->capacity = new_capacity; 
    //     canary_arragement(stack,new_data); 
    // }

    return 0;
}

Elem_t pop (struct Stack* stack) { 
    STACK_OK(stack);
    if (stack->size == 0) {
        return STACK_IS_EMPTY;
    }
    Elem_t new_size = stack->size - 1;
    Elem_t tmp = stack->data[new_size]; 
    stack->data[new_size] = POISON;
    stack->size -= 1;
    STACK_OK(stack);
    return tmp;
}

void stack_ctor(struct Stack* stack, int size, 
   const char* stack_name,
   const char* file_born,
   const char* func_born, int line) {
    assert(stack);

    stack->stack_name = stack_name;
    stack->file_born  = file_born;
    stack->func_born  = func_born;
    stack->line       = line;
    // if (stack->ctor_status == 1) { 
    //    return;
    // }
    // else {
        Canary_t* canary_1 = (Canary_t*)calloc(size * sizeof(Elem_t) + 2 * sizeof(Canary_t), sizeof(char));
        if (canary_1 == nullptr) {
            abort();
        }

        *canary_1 = CANARY;

        assert(canary_1);

        stack->data = (Elem_t*)(canary_1 + 1);

        assert(stack->data);

        Canary_t* canary_2 = (Canary_t*)((Elem_t*)canary_1 + size) + 1;
        *canary_2 = CANARY;

        stack->capacity = size;
        stack->size     = 0;

        stack->canary_left  = CANARY;
        stack->canary_right = CANARY;
        stack->ctor_status +=1;
    //}
}

void stack_dtor(struct Stack* stack) {
    if (stack == nullptr) return;

    if ((stack->canary_left != CANARY) || (stack->canary_right != CANARY)) {
        for (int i = 0; i < stack->capacity; i++) {
            stack->data[i] = POISON;
        }   
    }
      

    if (stack->data != nullptr) {
        free((Canary_t*)(stack->data - 1));
    }
     
    stack->size = 0; 
    stack->capacity = 0;
    stack->data = nullptr;  
}

void stack_dump(struct Stack* stack, FILE* fp, 
                const char* file_born, const char* func_born, int line) {
    assert(stack);
    assert(fp);

    fprintf(fp, "%s at %s(%d):\n", func_born, file_born, line);
    fprintf(fp, "\n");
    fprintf(fp,"Stack [%p] \"%s\" at %s() at %s(%d)\n", stack, stack->stack_name, stack->func_born, 
                                                        stack->file_born, stack->line);
    fprintf(fp, "\n");

    fprintf(fp, "canary_left = %d\n",  stack->canary_left);
    fprintf(fp, "data = %p\n",         stack->data);
    fprintf(fp, "size = %d\n",         stack->size);
    fprintf(fp, "capacity = %d\n",     stack->capacity);
    fprintf(fp, "ctor_status = %d\n",  stack->ctor_status);
    fprintf(fp, "err = %d\n",          stack->err);
    fprintf(fp, "canary_right = %d\n", stack->canary_right);


    if (stack->data) {
        fprintf(fp,"data_canary_left = %d\n", *((Canary_t*)stack->data - 1));
        for (int i = 0; i < stack->capacity; i++) {
            fprintf(fp, "stack->data[%d] = %d\n", i , stack->data[i]);
        }
        fprintf(fp,"data_canary_right = %d\n", stack->data[stack->capacity]);
    }

    fflush(fp);    
}

void canary_arragement(struct Stack* stack, Elem_t* new_data) {
    assert(stack);
    assert(new_data);

    Canary_t* canary_1 = (Canary_t*)new_data;
    *canary_1 = CANARY;

    stack->data = (Elem_t*)(canary_1 + 1); 

    Canary_t* canary_2 = (Canary_t*)(stack->data + stack->capacity);
    *canary_2 = CANARY;
}

int stack_is_destroyed(struct Stack* stack) {
    if ((stack != nullptr) && (stack->size == 0) &&
        (stack->capacity == 0)) {
            return 1;
        }
    return 0;
}