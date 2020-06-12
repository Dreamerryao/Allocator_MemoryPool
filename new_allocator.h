#ifndef NEW_ALLOCATOR_H
#define NEW_ALLOCATOR_H

#include <new>
#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <climits>
//allocator using new and delete
template <class T>
class New_Allocator
{
public:
    //some define from demo
    typedef T value_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t diference_type;
    typedef std::false_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;
    template <class U>
    struct rebind
    {
        typedef New_Allocator<U> other;
    };
    //construct
    New_Allocator()  = default;
    New_Allocator(const New_Allocator &New_Allocator) = default;
    New_Allocator(New_Allocator &&New_Allocator) = default;
    template <class U>
    New_Allocator(const New_Allocator<U> &other) noexcept;
    //new operator for allocate
    T* allocate(size_type n) {
        auto tmp = (T*)(::operator new((size_type)(n * sizeof(T))));

        if (tmp == 0)throw std::bad_alloc();
        return tmp;
    }
    //delete operator foe deallocate
    void deallocate(T* tmp, size_type) {delete(tmp); }

};
#endif