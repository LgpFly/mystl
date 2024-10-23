#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define THROW_BAD_ALLOC fprintf(stderr, "out of memroy\n"); exit(1);

namespace lgp
{
/*
 * 第一级配置器
 */
template<int inst>
class first_level_alloc
{
private:
    // 处理malloc异常
    static void* handler_oom_malloc(size_t);
    // 处理realloc异常
    static void* handler_oom_realloc(void*, size_t);
    // 客户端注册的处理函数，需要填充这个值
    static void(*real_handler_func)();

public:
    static void* allocate(size_t n)
    {
        void* result = malloc(n);
        if(0 == result)
            result = handler_oom_malloc(n);
        return result;
    }
    
    static void deallocate(void* p, size_t)
    {
        free(p);
    }

    static void* reallocate(void* p, size_t, size_t new_size)
    {
        void* result = realloc(p, new_size);
        if(0 == result)
            result = handler_oom_realloc(p, new_size);
        return result;
    }

    static void(* set_real_handler_func(void(*f)()))()
    {
        void (*old)() = read_handler_func;
        real_handler_func = f;
        return (old);
    }
};

template<int inst>
void (* first_level_alloc<inst>::real_handler_func)() = 0;

template<int inst>
void*
first_level_alloc<inst>::handler_oom_malloc(size_t n)
{
    void (*handler_malloc_func)();
    void* result;
    for( ; ; )
    {
        handler_malloc_func = real_handler_func;
        if(0 == handler_malloc_func) 
        {
            THROW_BAD_ALLOC;
        }
        (* handler_malloc_func)(); // 企图释放内存
        result = malloc(n);
        if(result)
            return result;
    }
}

template<int inst>
void*
first_level_alloc<inst>::handler_oom_realloc(void* p, size_t new_size)
{
    void (*handler_malloc_func)();
    void* result;
    
    for( ; ; )
    {
        handler_malloc_func = real_handler_func;
        if(0 == handler_malloc_func)
            THROW_BAD_ALLOC;
        (*handler_malloc_func)(); // 企图释放内存
        result = realloc(p, new_size);
        if(result) return result;
    }
}

typedef first_level_alloc<0> malloc_alloc;


class second_level_alloc
{
private:
    enum {ALIGN = 8};
    enum {MAX_BYTES = 128};
    enum {FREELISTS = 16};

    union obj
    {
        union obj*  next;
        char        data[1];
    };

    static char* s_start_free;
    static char* s_end_free;
    static size_t s_heap_size;

    static obj* volatile free_list[FREELISTS];
    
    


};









template<class T, class Alloc>
class simple_alloc
{
    static T* allocate(size_t n)
    {
        return 0 == n ? 0 : (T*)Alloc::allocate(n * sizeof(T));
    }
    
    static T* allocate(void)
    {
        return (T*)Alloc::allocate(sizeof(T));
    }

    static void deallocate(T* p, size_t n)
    {
        if(0 != n) 
            Alloc::deallocate(p, n * sizeof(T));
    }
    static void deallocate(T* p)
    {
        Alloc::deallocate(p, sizeof(T));
    }

};

}; // namespace lgp



#endif // __ALLOC_H__