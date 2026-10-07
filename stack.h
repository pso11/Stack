#ifdef STACK_USE_DOUBLE
        typedef double stack_elem_t;
        #define SPECIFICATOR "%lg"
        #define POISON    NAN
        #define CHECK_VALUES_IF_POISON() isnan((float)((stk -> data + sizeof(unsigned long long) / sizeof(stack_elem_t))[size]))
#elif STACK_USE_INT
        typedef int stack_elem_t;
        #define SPECIFICATOR "%d"
        #define POISON    109    //валентный угол алкана
        #define CHECK_VALUES_IF_POISON() (stk -> data + sizeof(unsigned long long) / sizeof(stack_elem_t))[size] == POISON
#endif

enum error_t
{
    OK                             = 0,
    NULL_POINTER                   = 1,
    OUT_OF_BOUNDS                  = 2,
    NULL_POINTER_FROM_CALLOC       = 3,
    NO_STACK_INITTED               = 4,
    BAD_ALLOCATION                 = 5,
    WRONG_CAPACITY                 = 6,
    WRONG_SIZE                     = 7,
    INVALID_LEFT_STRUCTURE_CANARY  = 8,
    INVALID_RIGHT_STRUCTURE_CANARY = 9,
    INVALID_LEFT_STACK_CANARY      = 10,
    INVALID_RIGHT_STACK_CANARY     = 11,
    INVALID_STACK_HASH             = 12,
    INVALID_STRUCTURE_HASH         = 13
};

#ifdef STACK_DEBUG
    void stack_dump(const struct stack_t* stk);
    unsigned long long hash(unsigned char* data, int capacity);
    void fwrite_stars(size_t number, FILE* file);

    struct source_location         \
    {                              \
        size_t line;               \
        const char* name;          \
        const char* file;          \
        const char* call_file;     \
        size_t call_line;          \
        const char* function;      \
        const char* birth_function;\
        const char* val_name;      \
        error_t error_code;        \
    };

    #define FILL_DBG(error)                  \
        dump_file.line       = __LINE__ ;    \
        dump_file.file       = __FILE__;     \
        dump_file.function   = __FUNCTION__; \
        dump_file.error_code = error
    #define FILL_BIRTH_FUNCTION()                \
            dump_file.birth_function = __FUNCTION__;

    #define ASSERT_STACK(stk, stack_error_status) \
            if (stack_error_status)               \
            {                                     \
                dump_file.call_file = __FILE__ ;  \
                dump_file.call_line = __LINE__;   \
                dump_file.val_name = #stk;        \
                stack_dump(stk);                  \
                stack_destroy(stk);               \
                return 0;                         \
            }

    #define HASH_PROTECTION(stk)                                                                      \
            stk -> structure_hash = 0;                                                                \
            stk -> stack_hash = 0;                                                                    \
            stk -> structure_hash = hash((unsigned char*)stk, sizeof(stk));                           \
            stk -> stack_hash     = hash((unsigned char*)(stk -> data + CANARY_SIZE), (stk -> capacity) * sizeof(stack_elem_t))

    #define CANARY_PROTECTION(stk)                                                                       \
            *((unsigned long long*)stk -> data)  = stk -> stack_canary;                                  \
            *((unsigned long long*)(stk -> data + stk -> capacity + CANARY_SIZE))  = stk -> stack_canary

    #define CANARY_SIZE sizeof(unsigned long long) / sizeof(stack_elem_t)
#else
    #define FILL_BIRTH_FUNCTION() ;
    #define FILL_DBG(error) ;
    #define ASSERT_STACK(stk, stack_error_status)                                   \
    {                                                                               \
        if (stack_error_status)                                                     \
        {                                                                           \
            printf("Error in your program. Turn on -DSTACK_DEBUG to understand it");\
            fflush(stdout);                                                         \
            return 0;                                                               \
        }                                                                           \
    }
    #define CANARY_SIZE 0
    #define CANARY_PROTECTION(stk);
    #define HASH_PROTECTION(stk)  ;
    #define FILL_DBG(error) ;
#endif

#define STACK_VERYFICATION(stk)                      \
        stack_error_status = stack_veryficator(stk); \
        if (stack_error_status)                      \
        {                                            \
            FILL_DBG(stack_error_status);            \
            return stack_error_status;               \
        }

struct error
{
    const char* description;
    int code;
};

struct stack_t
{
    #ifdef STACK_DEBUG
    unsigned long long structure_canary_left = 0xDEADBEEF;
    #endif

    stack_elem_t* data;
    int capacity;
    int size;

    #ifdef STACK_DEBUG
    unsigned long long stack_hash;
    unsigned long long structure_hash;
    unsigned long long stack_canary = 0xBA0BAB;
    unsigned long long structure_canary_right = 0xDEADBEEF;
    #endif
};

#define INCREMENT 2
#define PART      0.25

error_t stack_veryficator(struct stack_t* stk);
error_t stack_init(struct stack_t* stk, size_t stack_size);
error_t stack_destroy(struct stack_t* stk);
error_t stack_push(struct stack_t* stk, double value);
error_t resize_down(struct stack_t* stk);
error_t resize_up(struct stack_t* stk);
error_t stack_pop(struct stack_t* stk);


