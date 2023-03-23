#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#define CANARY_GUARD 0
#define HASH_GUARD 1

typedef int Elem_t;
typedef int Canary_t;

#define SPEC_ELEM_T "%d"
#define SPEC_CANARY_T "%lx"

#if CANARY_GUARD
const Canary_t CANARY = 0xBEAF;
#endif

const Elem_t POISON = 0xDEAD;

struct Stack {
#if CANARY_GUARD
   Canary_t canary_left = CANARY;
#endif
   
   Elem_t* data = nullptr;
   int size = 0;
   int capacity = 0;
   int ctor_status = 0;

   const char* stack_name = nullptr;
   const char* file_born  = nullptr;
   const char* func_born  = nullptr;
   int         line_born  = 0;

#if HASH_GUARD
   unsigned int hash = 0;
#endif

#if CANARY_GUARD
   Canary_t canary_right = CANARY;
#endif
}; 

#define STACK_CTOR(stk, size) stack_ctor(stk, size, #stk, __FILE__, __func__, __LINE__)
#define STACK_DUMP(stk, file, error) stack_dump(stk, file, error, __FILE__, __func__, __LINE__)

enum Capacity {
   INCREASE_CAPACITY = 1,
   DECREASE_CAPACITY = 2,
};

enum Errors {
   NO_ERROR                   =  0,
   STK_NULLPTR                =  1,
   STACK_DESTROYED            =  2,
   DATA_NULLPTR               =  3,
   NEGATIV_CAPACITY           =  4,
   NEGATIVE_SIZE              =  5,
   SIZE_OVERFLOW              =  6,
   CAPACITY_NULL              =  7,
   STACK_UNDERFLOW            =  8,
   STACK_OVERFLOW             =  9,
   DATA_CANARY_LEFT_DIED      = 10,
   DATA_CANARY_RIGHT_DIED     = 11,
   DATA_IS_POISON             = 12,
   PADDING_IS_NO_POISON       = 13,
   OUT_OF_MEMORY_FOR_RECALLOC = 14,
   STACK_IS_EMPTY             = 15,
   STACK_CANT_BE_RECTOR       = 16,
   CANT_CALLOC_FOR_STACK      = 17,
   NEW_DATA_NULLPTR           = 18, 
   CANARY_1_NULL              = 19,
   STACK_WAS_CONSTRUCTED      = 20,
   STACK_CANT_HASH            = 21,
   STACK_IS_NULLPTR           = 22,
   HASH_IS_NOT_HASH           = 23,
};

//=============================================================================

int stack_ctor(struct Stack* stack, int size, 
               const char* stack_name, const char* file_born,
               const char* func_born, int line_born);

void stack_dtor(struct Stack* stack);

void stack_push (struct Stack* stack, Elem_t value);

Elem_t stack_pop (struct Stack* stack);

void stack_dump(const struct Stack* stack, FILE* fp, int error,
                const char* file, const char* func, int line);

int stack_check(struct Stack* stk);

int stack_is_destroyed(const struct Stack* stack);

void poison_filling(struct Stack* stack);

unsigned int Hash_data(struct Stack* stack);

