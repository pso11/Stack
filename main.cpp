//имя файлв: номер строки
//записать хэши в дамп
//убрать канарейки в норм версии

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "main.h"
#include "stack.h"

#ifdef STACK_DEBUG
    struct source_location dump_file = {};
#endif

int main(void)
{
    error_t stack_error_status = OK;

    struct stack_t stack1 = {};
    FILL_BIRTH_FUNCTION();

    stack_error_status  = stack_init(&stack1, 25);
    ASSERT_STACK(&stack1, stack_error_status);

    stack_error_status  = stack_push(&stack1, 105);
    ASSERT_STACK(&stack1, stack_error_status);

    stack_error_status  = stack_pop(&stack1);
    ASSERT_STACK(&stack1, stack_error_status);

    //stack1.capacity = 20;
    //for (int i = 0; i < 20; i++)
    //{
    //    stack_error_status  = stack_push(&stack1, 105);
    //    ASSERT_STACK(&stack1, stack_error_status);
    //    printf("%d %d\n", stack1.size, stack1.capacity);
    //}

    stack_error_status  = stack_push(&stack1, 10);
    ASSERT_STACK(&stack1, stack_error_status);

    stack_error_status  = stack_push(&stack1, 10);
    ASSERT_STACK(&stack1, stack_error_status);

    stack_error_status  = stack_pop(&stack1);
    ASSERT_STACK(&stack1, stack_error_status);

    stack_error_status  = stack_pop(&stack1);
    ASSERT_STACK(&stack1, stack_error_status);

    stack_error_status  = stack_destroy(&stack1);
    ASSERT_STACK(&stack1, stack_error_status);

    return 0;
}

void imposter(void* array, int value, size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
        *((unsigned char*)array + i) = value;
}

void stack_print(const struct stack_t* stk)
{
    printf("\nPRINTING\nstack_t stk1 [%p] created by main(): \n"
           "capacity = [%d]\nsize = [%d]                     \n"
           "data [%p]\n{                                     \n",
            stk, stk -> capacity, stk -> size, stk -> data);

    for (int size = 0; size < stk -> capacity; size++)
    {
        printf("[%d] = " SPECIFICATOR " \n", size, stk -> data[size]);
        if (CHECK_VALUES_IF_POISON())
            printf("     <<POISON>>");
        putchar('\n');
    }

    putchar('}');
    putchar('\n');
}
