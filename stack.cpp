#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "stack.h"

#ifdef STACK_DEBUG
    extern struct source_location dump_file;
    const unsigned long long structure_canary_left  = 0xDEADBEEF;
    const unsigned long long structure_canary_right = 0xDEADBEEF;
#endif


error_t stack_init(struct stack_t* stk, size_t stack_size)
{
    error_t stack_error_status = OK;

    if (stk == NULL)
        return NO_STACK_INITTED;

    stk -> data     = (stack_elem_t*)calloc(stack_size + CANARY_SIZE * 2, sizeof(stack_elem_t));
    stk -> capacity = stack_size;
    stk -> size     = 0;

    CANARY_PROTECTION(stk);

    HASH_PROTECTION(stk);

    STACK_VERYFICATION(stk);

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

        (stk -> data + CANARY_SIZE)[size] = POISON;
    }

    HASH_PROTECTION(stk);
    STACK_VERYFICATION(stk);

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

    CANARY_PROTECTION(stk);

    HASH_PROTECTION(stk);

    STACK_VERYFICATION(stk);

    (stk -> data + CANARY_SIZE)[stk -> size++] = value;

    HASH_PROTECTION(stk);

    STACK_VERYFICATION(stk);

    return stack_error_status;
}

error_t stack_pop(struct stack_t* stk)
{
    error_t stack_error_status = OK;

    STACK_VERYFICATION(stk);

    (stk -> data + CANARY_SIZE)[--stk -> size] = POISON;

    HASH_PROTECTION(stk);
    STACK_VERYFICATION(stk);

    if (stack_error_status = resize_down(stk))
    {
        FILL_DBG(stack_error_status);
        return stack_error_status;
    }

    CANARY_PROTECTION(stk);
    HASH_PROTECTION(stk);

    STACK_VERYFICATION(stk);

    return stack_error_status;
}

error_t stack_veryficator(struct stack_t* stk)
{
    error_t stack_error_status = OK;

    if (stk == NULL)
        return NO_STACK_INITTED;

    #ifdef STACK_DEBUG
    if (hash((unsigned char*)(stk -> data + CANARY_SIZE), (stk -> capacity) * sizeof(stack_elem_t)) != stk -> stack_hash)
        return INVALID_STACK_HASH;

    unsigned long long structure_hash = stk -> structure_hash;
    unsigned long long stack_hash     = stk -> stack_hash;
    stk -> structure_hash = 0;
    stk -> stack_hash     = 0;

    if (hash((unsigned char*)stk, sizeof(stk)) != structure_hash)
    {
        stk -> stack_hash = stack_hash;
        stk -> structure_hash = structure_hash;
        return INVALID_STRUCTURE_HASH;
    }

    stk -> stack_hash = stack_hash;
    stk -> structure_hash = structure_hash;

    if (stk -> structure_canary_left != structure_canary_left)
        return INVALID_LEFT_STRUCTURE_CANARY;

    if (stk -> structure_canary_right != structure_canary_right)
        return INVALID_RIGHT_STRUCTURE_CANARY;

    if (stk -> stack_canary != *(unsigned long long*)(stk -> data))
        return INVALID_LEFT_STACK_CANARY;

    if (stk -> stack_canary != *(unsigned long long*)(stk -> data + stk -> capacity + CANARY_SIZE))
        return INVALID_RIGHT_STACK_CANARY;
    #endif

    if (stk -> data == NULL)
        return BAD_ALLOCATION;

    else if (stk -> capacity <= 0)
        return WRONG_CAPACITY;

    else if (stk -> size < 0)
        return WRONG_SIZE;

    else if (stk -> capacity <= stk -> size)
        return WRONG_SIZE;

    return stack_error_status;
}

error_t resize_up(struct stack_t* stk)
{
    error_t stack_error_status = OK;

    if (stk == NULL)
        return NO_STACK_INITTED;

    if (stk -> size + 1 == stk -> capacity)
    {
        stk -> capacity *= INCREMENT;

        stack_elem_t* temp = (stack_elem_t*)realloc(stk -> data, sizeof(stack_elem_t) * (stk -> capacity + 2 * CANARY_SIZE));
        if (temp != NULL)
        {
            stk -> data = temp;
            for (int i = stk -> size + 1; i < stk -> capacity; i++)
                (stk -> data + CANARY_SIZE)[i] = POISON;
        }
        else
            return NULL_POINTER_FROM_CALLOC;
    }

    return stack_error_status;

}

error_t resize_down(struct stack_t* stk)
{
    error_t stack_error_status = OK;

    if (stk == NULL)
        return NO_STACK_INITTED;

    if (((stk -> size) < PART * (stk -> capacity)) && (stk -> size != 0))
    {
        stk -> capacity = stk -> size + 1;

        stack_elem_t* temp = (stack_elem_t*)realloc(stk -> data, sizeof(stack_elem_t) * (stk -> capacity + 2 * CANARY_SIZE));
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

        (stk -> data + CANARY_SIZE)[size] = POISON;
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
        {"CU-CU-CU Something wrong with your right stack canary protection", 11},
        {"stack hash error -> !!Smth is trying to damage your stack!!", 12},
        {"structure hash error -> !!Smth is trying to damage your stack!!", 13}
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

    fprintf(file, "Error code: [%d] : <%s> called in: <%s()>:%d in function: <%s> : <%s>:%d\n\n"
                  "YOUR STACK: stack_t %s = [%p] created by %s():\n"
                  "capacity = [%d]\nsize = [%d]\n"
                  "data = [%p]\n"
                  "structure_canary_left  = [%llx]\n"
                  "structure_canary_right = [%llx]\n"
                  "stack     HASH = [%llx]\n"
                  "structure HASH = [%llx]\n{\n",
                   dump_file.error_code, array[structure].description, dump_file.call_file, dump_file.call_line,
                   dump_file.function, dump_file.file, dump_file.line, dump_file.val_name, stk, dump_file.birth_function,
                   stk -> capacity, stk -> size, stk -> data, stk -> structure_canary_left,
                   stk -> structure_canary_right, stk -> stack_hash, stk -> structure_hash);
    fflush(file);

    fprintf(file, "[%2d]  = %-20llx %s\n", -1, *(stk -> data), "<<CANARY>>");
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
    fprintf(file, "[%2d]  = %-20llx %s\n", stk -> capacity, *(stk -> data + stk -> capacity + sizeof(unsigned long long) / sizeof(stack_elem_t)), "<<CANARY>>");
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

unsigned long long hash(unsigned char* data, int capacity)
{
    unsigned long long hash = 5381;

    for (int i = 0; i < capacity; i++)
    {
        hash = ((hash << 5) + hash) + data[i];
    }

    return hash;
}

void fwrite_stars(size_t number, FILE* file)
{
    for (size_t i = 0; i < number; i++)
        fputc('*', file);
}
