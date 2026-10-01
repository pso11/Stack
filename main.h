#ifdef STACK_USE_DOUBLE
        typedef double stack_elem_t;
        #define SPECIFICATOR "%lg"
        #define POISON    NAN
        #define CHECK_VALUES_IF_POISON() isnan((float)(stk -> data[size]))
#elif STACK_USE_INT
        typedef int stack_elem_t;
        #define SPECIFICATOR "%d"
        #define POISON    10928    //валентный угол алкана
        #define CHECK_VALUES_IF_POISON() stk -> data[size] == POISON
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
    INVALID_RIGHT_STACK_CANARY     = 11

};

#ifdef STACK_DEBUG
    void stack_dump(const struct stack_t* stk);

    struct source_location         \
    {                              \
        size_t line;               \
        const char* name;          \
        const char* file;          \
        const char* function;      \
        const char* birth_function;\
        const char* val_name;      \
        error_t error_code;        \
    };

    #define FILL_BIRTH_FUNCTION()                \
            dump_file.birth_function = __FUNCTION__;

    #define ASSERT_STACK(stk, stack_error_status) \
            if (stack_error_status)               \
            {                                     \
                dump_file.val_name = #stk;        \
                stack_dump(stk);                  \
                stack_destroy(stk);               \
                return 0;                         \
            }

    #define FILL_DBG(error)       \
        dump_file.line       = __LINE__ - 2; \
        dump_file.file       = __FILE__;     \
        dump_file.function   = __FUNCTION__; \
        dump_file.error_code = error
#else
    #define FILL_BIRTH_FUNCTION() ;
    #define FILL_DBG(error) ;
    #define ASSERT_STACK(stk, stack_error_status) ;
#endif


struct error
{
    const char* description;
    int code;
};

struct stack_t
{
    unsigned long long structure_canary_left = 0xDEADBEEF;
    stack_elem_t* data;
    int capacity;
    int size;
    stack_elem_t stack_canary = 0xBA0BAB;
    unsigned long long structure_canary_right = 0xDEADBEEF;
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
void imposter(void* array, int value, size_t bytes);
void stack_print(const struct source_location* stk);
void fwrite_stars(size_t number, FILE* file);
