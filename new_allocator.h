#ifndef NEW_ALLOCATOR_H
#define NEW_ALLOCATOR_H

#include <new>
#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <climits>
//allocator using new and delete
template <class T>
class Malloc_Allocator
{
public:
    //some define from demo
    typedef T value_type;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef std::size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type is_always_equal;
    template <typename U>
    struct rebind
    {
        typedef Malloc_Allocator<U> other;
    };

    //construct
    Malloc_Allocator()  = default;
    Malloc_Allocator(const Malloc_Allocator &Malloc_Allocator) = default;
    Malloc_Allocator(Malloc_Allocator &&Malloc_Allocator) = default;
    //construct with template
    template <typename U>
    Malloc_Allocator(const Malloc_Allocator<U> &other) noexcept;
    //new operator for allocate
    T* allocate(std::size_t n) {
        //use new to create new space
        auto tmp = (T*)(::operator new((std::size_t)(n * sizeof(T))));
        //have no enough space throw bad_alloc
        if (tmp == 0)throw std::bad_alloc();
        return tmp;
    }

    //delete operator foe deallocate
    void deallocate(T* tmp, size_type) { ::operator delete(tmp); }

};

#endif