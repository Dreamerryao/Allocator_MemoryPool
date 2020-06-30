#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H

#include <iostream>

//some define 
const int MAX_BYTES = 65536;
const int BLOCK_SIZE =64;
const int FREELIST_NUM = MAX_BYTES / BLOCK_SIZE;

//union freelist_node
union fl_node {
    union fl_node *fl_ptr;//free_list_pointer
    char client_data[1];
};

class MemoryPool
{
private:
    //some define on http://web.mit.edu/cygwin/include/g++/stl_alloc.h
    static fl_node *free_list[FREELIST_NUM];
    //chunk allocation state
    static char *start_mp;
    static char *end_mp;
    static std::size_t heap_size;
    //ROUND_UP:function to align the block
    static std::size_t ROUND_UP(std::size_t bytes)
    {
        return (((bytes) + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1));
    }
    //FREELIST_INDEX:function to determine the index of free list what we will use
    static std::size_t FREELIST_INDEX(std::size_t bytes)
    {
        return ((bytes) + BLOCK_SIZE - 1) / BLOCK_SIZE - 1;
    }
    // return an object with the space of n
    static void *refill(std::size_t n){
        int cnt_flnodes = 16;//the number of nodes added each time
        char *chunk = chunk_alloc(n, cnt_flnodes);//get n bytes blocks from the memory pool
        fl_node ** now_list;//pointer to now free_list
		fl_node * res;
		fl_node *current_node, *next_node;

		size_t i;
		//only get one block
		if (cnt_flnodes==1) return chunk;
		//update the free list
		now_list = free_list + FREELIST_INDEX(n);
		// first block return 
		res = (fl_node*)chunk;
		*now_list = next_node = (fl_node*)(chunk + n);
        //get link for block left
		for (i = 1; i<cnt_flnodes; i++){
			current_node = next_node;
			next_node = (fl_node*)((char*)next_node + n);
			if (cnt_flnodes - 1 == i){
				current_node->fl_ptr = nullptr;
                break;
			}
			else{
				current_node->fl_ptr= next_node;
			}
		}

		return res;
    }

    //Use memory pool optimization
    static char *chunk_alloc(std::size_t size, int &cnt_flnodes)
    {
        char *res; //result
        // space we will use
        std::size_t total_size = size * cnt_flnodes;
        // unused space in memory pool
        std::size_t space_left = end_mp - start_mp;
        if (space_left >= total_size)
        { //no need to update space
            res = start_mp;
            start_mp += total_size;
            return res;
        }
        else if (space_left >= size)
        {
            // the remaining space in the memory pool is available for partial
            cnt_flnodes = space_left / size;
            total_size = size * cnt_flnodes;
            res = start_mp;
            start_mp += total_size;
            return res;
        }
        else
        {
            // the remaining space in the memory pool cannot be provided for one block
            //apply for memory from the system heap
            std::size_t bytes_to_get = 2 * total_size + ROUND_UP(heap_size >> 4);
            // try to make use of the scattered space
            if (bytes_to_get > 0 && start_mp != nullptr)
            {
                // if there is still some space
                fl_node **now_free_list =
                    free_list + FREELIST_INDEX(space_left);
                // update the free list
                ((fl_node *)start_mp)->fl_ptr = *now_free_list;
                *now_free_list = (fl_node *)start_mp;
            }

            // malloc new space
            start_mp = (char *)malloc(bytes_to_get);
            heap_size += bytes_to_get;
            end_mp = start_mp + bytes_to_get;

            // recursive
            return chunk_alloc(size, cnt_flnodes);
        }
    }

public:
    static void *allocate(std::size_t n){
        if(n > MAX_BYTES){
            return malloc(n);
        }
        fl_node ** now_fl = free_list + FREELIST_INDEX(n);
        fl_node *res = *now_fl;
        //if res = NULL refill the free list
        if(res == nullptr){
            return refill(ROUND_UP(n));
        }

        //update free list
        *now_fl = res->fl_ptr;
        return res;
    }

    static void deallocate(void *p, std::size_t n){
        if(n > MAX_BYTES){
            free(p);//if the requested memory is greater than MAX_BYTES, call free directly
            return;
        }
        //else put in free list
        fl_node* hhh = (fl_node*) p;
        fl_node** now_fl = free_list + FREELIST_INDEX(n);
        hhh->fl_ptr = * now_fl;
        *now_fl = hhh;
    }

    static void *reallocate(void *p, std::size_t old_sz, std::size_t new_sz){
        if(old_sz > MAX_BYTES){
            void * res = realloc(p,new_sz);
            return res;
        }
        else{
            deallocate(p,old_sz);
            return allocate(new_sz);
        }
    }
};

// initialize MemoryPool
char *MemoryPool::start_mp = nullptr;
char *MemoryPool::end_mp = nullptr;
std::size_t MemoryPool::heap_size = 0;
fl_node * MemoryPool::free_list[FREELIST_NUM] = {nullptr};
// mempory_allocator
template <class T>
class MyAllocator
{
public:
    //some define on https://en.cppreference.com/w/cpp/memory/allocator
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
        typedef MyAllocator<U> Other;
    };
    //constructor
    MyAllocator() = default;
    MyAllocator(const MyAllocator &myallocator) = default;
    MyAllocator(MyAllocator &&myallocator) = default;
    template <class U>
    MyAllocator(const MyAllocator<U> &myallocator) noexcept;
    //compute address
    pointer address(reference _Val) const noexcept{//return address of value
        return &_Val;
    }
    const_pointer address(const_reference _Val) const noexcept{
        return &_Val;
    }
    //deallocate
    void deallocate(pointer _Ptr, size_type _Count){
        MemoryPool::deallocate(_Ptr,_Count);
    }
    //use memortpool to allocate
    pointer allocate(size_type _Count) {
        if(auto res = static_cast<pointer>(MemoryPool::allocate(_Count * sizeof(value_type))))
        return res;
        else throw std::bad_alloc();
    }
    template <class _Uty>
    void destroy(_Uty *_Ptr){
        _Ptr->~_Uty();
    } 
    template <class _Objty, class... _Types>
    void construct(_Objty *_Ptr, _Types &&... _Args){
    new(_Ptr) _Objty(std::forward<_Types>(_Args)...);
    return;
    }
};
#endif