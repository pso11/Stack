//отслеживать уже заполненный стэк
//засунуть верификатор в дебаг версию

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "main.h"

#ifdef STACK_DEBUG
struct source_location dump_file = {};
#endif
const unsigned long long structure_canary_left  = 0xDEADBEEF;
const unsigned long long structure_canary_right = 0xDEADBEEF;
const unsigned long long stack_canary  = 0xBA0BAB;

int main(void)
{
    error_t stack_error_status = OK;

    struct stack_t stk1 = {};
    FILL_BIRTH_FUNCTION();

    stack_error_status  = stack_init(&stk1, 10);
    ASSERT_STACK(&stk1, stack_error_status);

    stack_error_status  = stack_push(&stk1, 105);
    ASSERT_STACK(&stk1, stack_error_status);
    imposter(stk1.data, 10, 8);

    stack_error_status  = stack_pop(&stk1);
    ASSERT_STACK(&stk1, stack_error_status);

    stack_error_status  = stack_push(&stk1, 10);
    ASSERT_STACK(&stk1, stack_error_status);

    stack_error_status  = stack_push(&stk1, 10);
    ASSERT_STACK(&stk1, stack_error_status);

    stack_error_status  = stack_pop(&stk1);
    ASSERT_STACK(&stk1, stack_error_status);

    stack_error_status  = stack_pop(&stk1);
    ASSERT_STACK(&stk1, stack_error_status);

    stack_error_status  = stack_destroy(&stk1);
    ASSERT_STACK(&stk1, stack_error_status);

    return 0;
}

error_t stack_init(struct stack_t* stk, size_t stack_size)
{
    error_t stack_error_status = OK;

    stk -> data     = (stack_elem_t*)calloc(stack_size + (sizeof(unsigned long long) / sizeof(stack_elem_t)) * 2, sizeof(stack_elem_t));
    printf("%d", stack_size + (sizeof(unsigned long long) / sizeof(stack_elem_t)));
    stk -> capacity = stack_size;
    stk -> size     = 0;

    *((unsigned long long*)stk -> data)  = stk -> stack_canary;
    *((unsigned long long*)(stk -> data + stk -> capacity + (sizeof(unsigned long long) / sizeof(stack_elem_t))))  = stk -> stack_canary;

    stack_error_status = stack_veryficator(stk);
    if (stack_error_status)
    {
        FILL_DBG(stack_error_status);
        return stack_error_status;
    }

    for (int size = 0; size < stk -> capacity; size++)
    {
        if (size < 0)
        {
            FILL_DBG(OUT_OF_BOUNDS);
            return OUT_OF_BOUNDS;
        }

        if (size >= stk -> capacity)
        {
            FILL_DBG(OUT_OF_BOUNDS);
            return OUT_OF_BOUNDS;
        }

        (stk -> data + sizeof(unsigned long long) / sizeof(stack_elem_t))[size] = POISON;
    }

    stack_error_status = stack_veryficator(stk);
    FILL_DBG(stack_error_status);
    return stack_error_status;
}

error_t stack_push(struct stack_t* stk, double value)
{
    error_t stack_error_status = OK;

    stack_error_status = resize_up(stk);
    if (stack_error_status)
    {
        FILL_DBG(stack_error_status);
        return stack_error_status;
    }

    stack_error_status = stack_veryficator(stk);
    if (stack_error_status)
    {
        FILL_DBG(stack_error_status);
        return stack_error_status;
    }

    (stk -> data + sizeof(unsigned long long) / sizeof(stack_elem_t))[stk -> size++] = value;

    stack_error_status = stack_veryficator(stk);
    FILL_DBG(stack_error_status);
    return stack_error_status;
}

error_t stack_pop(struct stack_t* stk)
{
    error_t stack_error_status = OK;

    stack_error_status = stack_veryficator(stk);
    if (stack_error_status)
    {
        FILL_DBG(stack_error_status);
        return stack_error_status;
    }

    (stk -> data + sizeof(unsigned long long) / sizeof(stack_elem_t))[--stk -> size] = POISON;

    stack_error_status = stack_veryficator(stk);
    if (stack_error_status)
    {
        FILL_DBG(stack_error_status);
        return stack_error_status;
    }

    stack_error_status = resize_down(stk);
    {
        FILL_DBG(stack_error_status);
        return stack_error_status;
    }

    stack_error_status = stack_veryficator(stk);
    FILL_DBG(stack_error_status);
    return stack_error_status;
}

error_t stack_veryficator(struct stack_t* stk)
{
    error_t stack_error_status = OK;

    if (stk == NULL)
        return NO_STACK_INITTED;

    if (stk -> data == NULL)
        return BAD_ALLOCATION;

    else if (stk -> capacity <= 0)
        return WRONG_CAPACITY;

    else if (stk -> size < 0)
        return WRONG_SIZE;

    else if (stk -> capacity <= stk -> size)
        return WRONG_SIZE;

    if (memcmp(&(stk -> structure_canary_left), &structure_canary_left, sizeof(unsigned long long)) != 0)
        return INVALID_LEFT_STRUCTURE_CANARY;
    if (memcmp(&(stk -> structure_canary_right), &structure_canary_right, sizeof(unsigned long long)) != 0)
        return INVALID_RIGHT_STRUCTURE_CANARY;
    if (memcmp(&(stk -> stack_canary), stk -> data, sizeof(unsigned long long)) != 0)
        return INVALID_LEFT_STACK_CANARY;
    if (memcmp(&(stk -> stack_canary), stk -> data + stk -> capacity + sizeof(unsigned long long) / sizeof(stack_elem_t), sizeof(unsigned long long)) != 0)
        return INVALID_RIGHT_STACK_CANARY;

    return stack_error_status;

}

error_t resize_up(struct stack_t* stk)
{
    error_t stack_error_status = OK;

    if (stk -> size + 1 == stk -> capacity)
    {
        stk -> capacity *= INCREMENT;

        stack_elem_t* temp = (stack_elem_t*)realloc(stk -> data, sizeof(stack_elem_t) * (stk -> capacity + 2 * (sizeof(unsigned long long) / sizeof(stack_elem_t))));
        if (temp != NULL)
        {
            stk -> data = temp;
            for (int i = stk -> size + 1; i < stk -> capacity; i++)
                (stk -> data + sizeof(unsigned long long) / sizeof(stack_elem_t))[i] = POISON;
        }
        else
            return NULL_POINTER_FROM_CALLOC;
    }

    return stack_error_status;

}

error_t resize_down(struct stack_t* stk)
{
    error_t stack_error_status = OK;

    if (((stk -> size) < PART * (stk -> capacity)) && (stk -> size != 0))
    {
        stk -> capacity = stk -> size + 1;

        stack_elem_t* temp = (stack_elem_t*)realloc(stk -> data, sizeof(stack_elem_t) * (stk -> capacity + 2 * (sizeof(unsigned long long) / sizeof(stack_elem_t))));
        if (temp != NULL)
            stk -> data = temp;
        else
            return NULL_POINTER_FROM_CALLOC;
    }

    return stack_error_status;
}

error_t stack_destroy(struct stack_t* stk)
{
    error_t stack_error_status = OK;

    if (stk == NULL)
        return NO_STACK_INITTED;

    for (int size = 0; size < stk -> capacity; size++)
    {
        if (size < 0)
        {
            FILL_DBG(OUT_OF_BOUNDS);
            return OUT_OF_BOUNDS;
        }

        if (size >= stk -> capacity)
        {
            FILL_DBG(OUT_OF_BOUNDS);
            return OUT_OF_BOUNDS;
        }

        (stk -> data + sizeof(unsigned long long) / sizeof(stack_elem_t))[size] = POISON;
    }

    free(stk -> data);
    stk = NULL;
    stk -> data = NULL;

    return stack_error_status;
}

#ifdef STACK_DEBUG

void stack_dump(const struct stack_t* stk)
{
    struct error array[] =
    {
        {"Meet NULL pointer", 1},
        {"Index out of array -> memory leak", 2},
        {"Realloc didn't find place to replace stack", 3},
        {"Wrong NULL poiner on stack", 4},
        {"Allocation didn't work -> NO stack inited", 5},
        {"Capacity less than ZERO", 6},
        {"Wrong value of size: more than capacity \\ less than ZERO", 7},
        {"CU-CU-CU Something wrong with your left structure canary protection", 8},
        {"CU-CU-CU Something wrong with your right structure canary protection", 9},
        {"CU-CU-CU Something wrong with your left stack canary protection", 10},
        {"CU-CU-CU Something wrong with your right stack canary protection", 11}
    };

    printf("Hiiii!!!! LOOK AT MAIN.LOG!!!");
    FILE* file = fopen("main.log", "a+");

    fputc('\n', file);
    fwrite_stars(25, file);
    fprintf(file,"\nMEET ERROR IN YOUR PROGRAMM READ ALL INFORMATION DOWN\n");
    fwrite_stars(25, file);
    fputc('\n', file);

    size_t structure = 0;
    while (structure < sizeof(array) / sizeof(array[0]))
    {
        if (array[structure].code == dump_file.error_code)
            break;
        structure++;
    }

    fprintf(file, "Error code: [%d] : <%s> in file: <%s> in function: <%s()> in line: [%d]\n\n"
                  "YOUR STACK: stack_t %s = [%p] created by %s():\n"
                  "capacity = [%d]\nsize = [%d]\n"
                  "&data = [%p]\n{\n",
                   dump_file.error_code, array[structure].description, dump_file.file, dump_file.function,
                   dump_file.line, dump_file.val_name, stk, dump_file.birth_function, stk -> capacity, stk -> size, stk -> data);
    fflush(file);

    fprintf(file, "[%2d]  = %llx           <<CANARY>>\n", -1, *(stk -> data));
    for (int size = 0; size < stk -> capacity; size++)
    {
        fprintf(file,"[%2d]  = [" SPECIFICATOR "]", size, (stk -> data + sizeof(unsigned long long) / sizeof(stack_elem_t))[size]);
        if (CHECK_VALUES_IF_POISON())
        {
            fprintf(file, "     <<POISON>>");
            fflush(file);
        }
        fputc('\n', file);
    }
    fprintf(file, "[%2d]  = %llx           <<CANARY>>\n", stk -> capacity, *(stk -> data + stk -> capacity + sizeof(unsigned long long) / sizeof(stack_elem_t)));
    fputc('}', file);
    fputc('\n', file);

    fputc('\n', file);
    fwrite_stars(25, file);
    fprintf(file,"\nEND OF OUR MESSAGE\n");
    fwrite_stars(25, file);
    fputc('\n', file);
    fputc('\n', file);

    fclose(file);
}

#endif

void imposter(void* array, int value, size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
        *((unsigned char*)array + i) = value;
}

void stack_print(const struct stack_t* stk)
{
    printf("\nPRINTING\nstack_t stk1 [%p] created by main(): \n"
           "capacity = [%d]\nsize = [%d]                    \n"
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

void fwrite_stars(size_t number, FILE* file)
{
    for (size_t i = 0; i < number; i++)
        fputc('*', file);
}
