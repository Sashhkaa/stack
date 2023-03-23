#include "stack.h"
#include <math.h>

#define CASE_ERROR_TEXT(enum_const) case enum_const: return #enum_const;

#define STACK_OK(stk)                            \
    do {                                          \
        int error = stack_check(stk);              \
        if (error != NO_ERROR) {                                    \
            FILE* err_file = fopen("err.txt", "a");  \
            STACK_DUMP(stk, err_file, error);         \
            fclose(err_file);                          \
            abort();                                    \
        }                                                \
    } while (0)

//=============================================================================

Elem_t stack_resize(struct Stack* stack,int mode);

void canary_arragement(struct Stack* stack, Elem_t* new_data);

const char* text_errors(const int error);

void poison_filling(struct Stack* stack);

unsigned int Hash_data(struct Stack* stack);

//=============================================================================


int stack_ctor(struct Stack* stack, int size, 
                const char* stack_name, const char* file_born,
                const char* func_born, int line_born) {
    assert(stack);

    if (stack->ctor_status == 1) { 
       return STACK_WAS_CONSTRUCTED;
    }
    else {    
        stack->stack_name = stack_name;
        stack->file_born  = file_born;
        stack->func_born  = func_born;
        stack->line_born  = line_born;

#if CANARY_GUARD
        Canary_t* canary_1 = (Canary_t*)calloc(size * sizeof(Elem_t) + 2 * sizeof(Canary_t), sizeof(char));
        if (canary_1 == nullptr) {
            return CANARY_1_NULL;
        }

        *canary_1 = CANARY;

        Canary_t* canary_2 = (Canary_t*)((Elem_t*)canary_1 + size) + 1;
        *canary_2 = CANARY;

        stack->data = (Elem_t*)(canary_1 + 1);

        stack->canary_left  = CANARY;
        stack->canary_right = CANARY;

#else
        stack->data = (Elem_t*)calloc(size * sizeof(Elem_t), sizeof(char));
        if (stack->data == nullptr) {
            return DATA_NULLPTR;
        }
#endif // CANARY_GUARD

        stack->capacity = size;
        stack->size     = 0;
        stack->ctor_status += 1; 

        for (int i = 0; i < stack->capacity; i++) {
            stack->data[i] = POISON;
        } 

#if HASH_GUARD
        stack->hash = Hash_data(stack);
#endif // HASH_GUARD
    }

    return 0;
}

void stack_dtor(struct Stack* stack) {
    if (stack == nullptr) return;

#if CANARY_GUARD
    if (stack->data != nullptr) {
        if ((stack->canary_left != CANARY) || (stack->canary_right != CANARY)) {
            for (int i = 0; i < stack->capacity; i++) {
                stack->data[i] = POISON;
            }   
        }

        free((Canary_t*)(stack->data - 1));
    }
#else 
    if (stack->data != nullptr) { {
            for (int i = 0; i < stack->capacity; i++) {
                stack->data[i] = POISON;
            }   
        }

        free(stack->data);
    }
#endif // CANARY_GUARD

#if HASH_GUARD
    stack->hash = 0;
#endif // HASH_GUARD

    stack->size = 0; 
    stack->capacity = 0;
    stack->data = nullptr;
    stack->ctor_status = 0;
}

void stack_push(struct Stack* stack, Elem_t value) { 
    STACK_OK(stack);

    if (stack->size >= stack->capacity) {
        stack_resize(stack, INCREASE_CAPACITY);
    }

    stack->data[stack->size++] = value;

    stack->hash = Hash_data(stack);
    STACK_OK(stack);
}

Elem_t stack_pop(struct Stack* stack) { 
    STACK_OK(stack);

    if (stack->size == 0) {
        return STACK_IS_EMPTY;
    }
    if (stack->size < stack->capacity / 4) {
        stack_resize(stack, DECREASE_CAPACITY);
    }

    Elem_t elem = stack->data[stack->size]; 
    stack->data[--stack->size] = POISON;

    stack->hash = Hash_data(stack);
    STACK_OK(stack);
    return elem;
}

int is_poison(Elem_t data) {
    return (data == POISON);
}

void stack_dump(const struct Stack* stack, FILE* fp, int error,
                const char* file, const char* func, int line) {
    assert(stack);
    assert(fp);

    const char* code = text_errors(error);
    
    fprintf(fp, "error = %s %s at %s(%d):\n", code, func, file, line);
    fprintf(fp, "Stack [%p] \"%s\" at %s() at %s(%d)\n", stack, stack->stack_name, stack->func_born, 
                                                         stack->file_born, stack->line_born);
    fprintf(fp, "\n");

#if CANARY_GUARD
    fprintf(fp, "canary_left = " SPEC_CANARY_T "\n",  stack->canary_left);
#endif // CANARY_GUARD

    fprintf(fp, "data = %p\n",         stack->data);
    fprintf(fp, "size = %d\n",         stack->size);
    fprintf(fp, "capacity = %d\n",     stack->capacity);
    fprintf(fp, "ctor_status = %d\n",  stack->ctor_status);

#if CANARY_GUARD
    fprintf(fp, "canary_right = " SPEC_CANARY_T "\n", stack->canary_right);
#endif // CANARY_GUARD

    if (stack->data) {
#if CANARY_GUARD
        fprintf(fp,"data_canary_left = " SPEC_CANARY_T "\n", *((Canary_t*)stack->data - 1));
#endif // CANARY_GUARD

        for (int i = 0; i < stack->size; i++) {
            if (is_poison(stack->data[i])) {
                fprintf(fp, "* stack->data[%d] = " SPEC_ELEM_T " (POISON!)\n", i , stack->data[i]);
            }
            else {
                fprintf(fp, "* stack->data[%d] = " SPEC_ELEM_T "\n", i , stack->data[i]);
            }
        }

        for (int i = stack->size; i < stack->capacity; i ++){
            if (is_poison(stack->data[i])) {
                fprintf(fp, "  stack->data[%d] = " SPEC_ELEM_T " (POISON!)\n", i , stack->data[i]);
            }
            else {
                fprintf(fp, "  stack->data[%d] = " SPEC_ELEM_T "\n", i , stack->data[i]);
            }
        }

#if CANARY_GUARD
        fprintf(fp, "data_canary_right = " SPEC_CANARY_T "\n", stack->data[stack->capacity]);
#endif // CANARY_GUARD
    }
    fprintf(fp, "\n");

    fflush(fp);    
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
    if (stack->size > stack->capacity) {
        return SIZE_OVERFLOW;
    }
    if (stack->capacity == 0) {
        return CAPACITY_NULL;
    }

#if CANARY_GUARD
    
    if (stack->canary_left  != CANARY) {
        return STACK_UNDERFLOW;
    }

    if (stack->canary_right != CANARY) {
        return STACK_OVERFLOW;
    }

    if (*((Canary_t*)stack->data- 1) != CANARY) {
        return DATA_CANARY_LEFT_DIED;
    }
    
    if (*((Canary_t*)(stack->data + stack->capacity)) != CANARY) {
        return DATA_CANARY_RIGHT_DIED;
    }
#endif // CANARY_GUARD
    unsigned int new_hash = stack->hash;
    stack->hash = 0; 
    if (new_hash != Hash_data(stack)) {
        return HASH_IS_NOT_HASH;
    }

    stack->hash = new_hash;
    
    for (int i = 0; i < stack->size; i++) { 
        if (stack->data[i] == POISON) {
           return DATA_IS_POISON;
        }
    }
    
    for (int i = stack->size; i < stack->capacity; i++) { 
        if (stack->data[i] != POISON) {
            return PADDING_IS_NO_POISON;
        }
    }

    return NO_ERROR;
}

int stack_is_destroyed(const struct Stack* stack) {
    if ((stack != nullptr) && (stack->size == 0) &&
        (stack->capacity == 0)) {
            return 1;
        }
    return 0;
}

//=============================================================================

Elem_t stack_resize(struct Stack* stack,int mode) { 

#if CANARY_GUARD 
    if (mode == INCREASE_CAPACITY) {
        Elem_t* new_data = stack->data;
        int new_capacity = stack->capacity * 2;
   
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
            poison_filling(stack);
        }
    }

    if (mode == DECREASE_CAPACITY) {
        Elem_t* new_data = (Canary_t*)stack->data - 1;
        int new_capacity = stack->capacity / 2;

        new_data = (Elem_t*)realloc(new_data, new_capacity * sizeof(Elem_t) + 2 * sizeof(Canary_t));
        if (new_data == nullptr) {
            return NEW_DATA_NULLPTR;
        }

        stack->capacity = new_capacity; 
        canary_arragement(stack,new_data); 
        poison_filling(stack);
    }
#else 
if (mode == INCREASE_CAPACITY) {
        Elem_t* new_data = stack->data;
        int new_capacity = stack->capacity * 2;
   
        new_data = (Elem_t*)realloc(new_data, new_capacity * sizeof(Elem_t));
        if (new_data == nullptr) {
            new_capacity = stack->capacity + 1;
            new_data = (Elem_t*)realloc(new_data, new_capacity * sizeof(Elem_t)); 
        }

        if (new_data == nullptr) {
            return OUT_OF_MEMORY_FOR_RECALLOC;
        }
        else {
            stack->capacity = new_capacity;
            poison_filling(stack);
        }
    }

    if (mode == DECREASE_CAPACITY) {
        Elem_t* new_data = stack->data;
        int new_capacity = stack->capacity / 2;

        new_data = (Elem_t*)realloc(new_data, new_capacity * sizeof(Elem_t));
        if (new_data == nullptr) {
            return NEW_DATA_NULLPTR;
        }

        stack->capacity = new_capacity; 
        poison_filling(stack);
    }

#endif // CANARY_GUARD

    return 0;
}

const char* text_errors(const int error) {

    switch (error) {
        CASE_ERROR_TEXT(NO_ERROR);
        CASE_ERROR_TEXT(STK_NULLPTR);            
        CASE_ERROR_TEXT(STACK_DESTROYED);        
        CASE_ERROR_TEXT(DATA_NULLPTR);           
        CASE_ERROR_TEXT(NEGATIV_CAPACITY);       
        CASE_ERROR_TEXT(NEGATIVE_SIZE);        
        CASE_ERROR_TEXT(SIZE_OVERFLOW);          
        CASE_ERROR_TEXT(CAPACITY_NULL);          
        CASE_ERROR_TEXT(STACK_UNDERFLOW);        
        CASE_ERROR_TEXT(STACK_OVERFLOW);
        CASE_ERROR_TEXT(DATA_IS_POISON); 
        CASE_ERROR_TEXT(PADDING_IS_NO_POISON);
        CASE_ERROR_TEXT(OUT_OF_MEMORY_FOR_RECALLOC);
        CASE_ERROR_TEXT(STACK_IS_EMPTY);
        CASE_ERROR_TEXT(STACK_CANT_BE_RECTOR);
        CASE_ERROR_TEXT(CANT_CALLOC_FOR_STACK);
        CASE_ERROR_TEXT(NEW_DATA_NULLPTR);
        CASE_ERROR_TEXT(CANARY_1_NULL);
        CASE_ERROR_TEXT(STACK_WAS_CONSTRUCTED);
        CASE_ERROR_TEXT(STACK_IS_NULLPTR);
        CASE_ERROR_TEXT(HASH_IS_NOT_HASH);
#if CANARY_GUARD
        CASE_ERROR_TEXT(STACK_CANT_HASH);
        CASE_ERROR_TEXT(DATA_CANARY_LEFT_DIED); 
        CASE_ERROR_TEXT(DATA_CANARY_RIGHT_DIED); 
        CASE_ERROR_TEXT(CANARY_1_NULL);  
#endif    
        default: return "Unknown ERROR";
    }
}

void poison_filling(struct Stack* stack) {
    assert(stack);

    for (int i = stack->size; i < stack->capacity; i++) { 
        stack->data[i] = POISON;
    }
}

#if CANARY_GUARD
void canary_arragement(struct Stack* stack, Elem_t* new_data) {
    assert(stack);
    assert(new_data);

    Canary_t* canary_1 = (Canary_t*)new_data;
    *canary_1 = CANARY;

    stack->data = (Elem_t*)(canary_1 + 1); 

    Canary_t* canary_2 = (Canary_t*)(stack->data + stack->capacity);
    *canary_2 = CANARY;
}
#endif //CANARY_GUARD

#if HASH_GUARD
unsigned int Hash_data(struct Stack* stack) {
    assert(stack);
    assert(stack->data);
    
    stack->hash = 0;
     
    unsigned int hash = 0;

    int max_size = sizeof(stack[0]);

    for (int i = 0; i < max_size; ++i) {
        hash +=  *((char*)stack + i);
        hash +=  (hash <<  10);
        hash ^=  (hash >>  6);
    }
    if (stack->data != nullptr) {
        for (int i = 0; i < stack->capacity * sizeof(Elem_t) ; ++i) {
            hash +=  *((char*)stack->data + i);
            hash +=  (hash << 10);
            hash ^=  (hash >>  6);
        }
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}
#endif //HASH_GUARD


 
