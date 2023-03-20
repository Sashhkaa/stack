#include <stdio.h>
#include <malloc.h>
#include <assert.h>

typedef int Elem_t;
typedef int Canary_t;

const Canary_t CANARY = 0xBEAF;
const Elem_t POISON = 0xDEAD;

struct Stack {
   Canary_t canary_left = CANARY;
   
   Elem_t* data = nullptr;
   int size = 0;
   int capacity = 0;
   int ctor_status = 0;

   int err = 0;

   const char* stack_name = nullptr;
   const char* file_born = nullptr;
   const char* func_born = nullptr;
   int line = 0;

   Canary_t canary_right = CANARY;
}; 

#define STACK_CTOR(stk, size) stack_ctor(stk, size, #stk, __FILE__, __func__, __LINE__)
#define STACK_DUMP(stk, file) stack_dump(stk, file, __FILE__, __func__, __LINE__)

enum Capacity {
   INCREASE_CAPACITY = 1,
   DECREASE_CAPACITY = 2,
};

enum Errors {
   NO_ERROR               = 0,
   STK_NULLPTR            = 1,
   STACK_DESTROYED        = 2,
   DATA_NULLPTR           = 3,
   NEGATIV_CAPACITY       = 4,
   NEGATIVE_SIZE          = 5,
   SIZE_OVERFLOW          = 6,
   CAPACITY_NULL          = 7,
   STACK_UNDERFLOW        = 8,
   STACK_OVERFLOW         = 9,
   DATA_CANARY_LEFT_DIED  = 10,
   DATA_CANARY_RIGHT_DIED = 11,
   DATA_IS_POISON         = 12,
   PADDING_IS_NO_POISON   = 13,
   OUT_OF_MEMORY_FOR_RECALLOC = 14,
   STACK_IS_EMPTY         = 15,
   STACK_CANT_BE_RECTOR   = 16,
   CANT_CALLOC_FOR_STACK  = 17, 
};
void stack_ctor(struct Stack* stack, int size, 
   const char* stack_name,
   const char* file_born,
   const char* func_born, int line);
void stack_dump(struct Stack* stack, FILE* fp,
   const char* file_born,
   const char* func_born, int line); 
int stack_check(const struct Stack* const stk);
void stack_push (struct Stack* stack, Elem_t value);
void Resize(struct Stack* stack);
Elem_t pop (struct Stack* const stack);
void stack_dtor(struct Stack* stack);