#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define THROW_BAD_ALLOC fprintf(stderr, "out of memroy\n"); exit(1);

namespace ministl
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

    // 这三个变量是由chunk_alloc 来更新的
    static char* s_start_free;
    static char* s_end_free;
    static size_t s_heap_size;

    static obj* volatile free_list[FREELISTS];

    // 调整至8的倍数
    static size_t round_up(size_t n){
        return ((n + (ALIGN-1)) & ~(ALIGN - 1));
    }

    // 寻找n对应的空闲链表的下表
    static size_t find_free_list_idx(size_t n){
        return (((n + (size_t)(ALIGN-1)) / (size_t)ALIGN) - 1);
    } 

    // 返回一块内存，填充空闲链表
    static void* refill(size_t n);

    // 配置一块大空间，nobjs*size(nobhs个size大小)
    static char* chunk_alloc(size_t size, int &nobjs);

public:
    static void* allocate(size_t n);
    static void deallocate(void* p, size_t n);
    static void* reallocate(void* p, size_t old_size, size_t new_size);
};

// 对second_level_alloc中静态变量进行赋值

char* second_level_alloc::s_start_free = 0;
char* second_level_alloc::s_end_free = 0;
size_t second_level_alloc::s_heap_size = 0;

second_level_alloc::obj* volatile second_level_alloc::free_list[FREELISTS] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void* second_level_alloc::allocate(size_t n){
    obj* volatile * my_free_list;
    obj* res;
    // 大于128直接调用一级配置器
    if(n > (size_t)(MAX_BYTES)){
        return(malloc_alloc::allocate(n));
    }

    // 找到相应的链表 如果为空调用refill填充并返回，否则直接返回结果
    my_free_list = free_list + find_free_list_idx(n);
    res = *my_free_list;

    if(res == 0){
        void* r = refill(round_up(n));
        return r;
    }

    *my_free_list = res->next;
    return(res);
}

// 填充空闲链表
void* second_level_alloc::refill(size_t n){
    // 默认填充20个
    int nobjs = 20;
    // 调用chunck_alloc 返回地址空间
    char* chunk = chunk_alloc(n, nobjs);

    if(1 == nobjs){
        return chunk;
    }

    obj* volatile* my_free_list = free_list + find_free_list_idx(n);
    obj* res = (obj*)(chunk);

    obj* next = 0;
    *my_free_list = next = (obj*)(chunk + n);
    for(int i = 1; ; i++){
        obj* cur = next;
        next = (obj*)((char*)(next) + n);
        if(nobjs - 1 == i){
            cur->next = 0;
            break;
        }else
            cur->next = next;
    }

    return res;
}











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

}; // namespace ministl



#endif // __ALLOC_H__