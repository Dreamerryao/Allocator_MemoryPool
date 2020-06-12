#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H
#include <cstddef>
#include <type_traits>
#include <climits>
#include <utility>

const size_t BlockSize = 65536;
template <typename T>
class MyAllocator
{
public:
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef const T* const_pointer;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef std::false_type propagate_on_container_copy_assignment;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type  propagate_on_container_swap;
	template <typename U> struct rebind
	{
		typedef MyAllocator<U> Other;
	};
	MyAllocator() noexcept;
	MyAllocator(const MyAllocator& myallocator) noexcept;
	MyAllocator(MyAllocator&& myallocator) noexcept;
	template <class U>MyAllocator(const MyAllocator<U>& myallocator) noexcept;
	~MyAllocator() noexcept;
	MyAllocator& operator=(const MyAllocator& myallocator) = delete;
	MyAllocator& operator=(MyAllocator&& myallocator) noexcept;
	pointer address(reference x) const noexcept;
	const_pointer address(const_reference x) const noexcept;
	pointer allocate(size_type n = 1, const_pointer hint = 0);
	void deallocate(pointer p, size_type n = 1);
	size_type max_size() const noexcept;
	template <class U, class... Args> void construct(U* p, Args&&... args);
	template <class U> void destroy(U* p);
	template <class... Args> pointer newElement(Args&&... args);
	void deleteElement(pointer p);
private:
	union Slot_
	{
		value_type element;
		Slot_* next;
	};
	typedef char* data_pointer_;
	typedef Slot_ slot_type_;
	typedef Slot_* slot_pointer_;
	slot_pointer_ currentBlock_;
	slot_pointer_ currentSlot_;
	slot_pointer_ lastSlot_;
	slot_pointer_ freeSlots_;
	size_type padPointer(data_pointer_ p, size_type align) const noexcept;
	void allocateBlock();
	static_assert(BlockSize >= 2 * sizeof(slot_type_), "Blocksize too small.");
};
template<typename T>
inline typename MyAllocator<T>::size_type MyAllocator<T>::padPointer(data_pointer_ p, size_type align) const noexcept
{
	uintptr_t result = reinterpret_cast<uintptr_t>(p);
	return ((align - result) % align);
}
template<typename T>
MyAllocator<T>::MyAllocator() noexcept
{
	currentBlock_ = nullptr;
	currentSlot_ = nullptr;
	lastSlot_ = nullptr;
	freeSlots_ = nullptr;
}
template<typename T>
MyAllocator<T>::MyAllocator(const MyAllocator & myallocator) noexcept:MyAllocator()
{}
template<typename T>
MyAllocator<T>::MyAllocator(MyAllocator && myallocator) noexcept
{
	currentBlock_ = myallocator.currentBlock_;
	myallocator.currentBlock_ = nullptr;
	currentSlot_ = myallocator.currentSlot_;
	lastSlot_ = myallocator.lastSlot_;
	freeSlots_ = myallocator.freeSlot_;
}
template<typename T>
template <class U>
MyAllocator<T>::MyAllocator(const MyAllocator<U>& myallocator) noexcept:MyAllocator()
{}
template<typename T>
MyAllocator<T> & MyAllocator<T>::operator=(MyAllocator && myallocator) noexcept
{
	if (this != &myallocator)
	{
		std::swap(currentBlock_, myallocator.currentBlock_);
		currentSlot_ = myallocator.currentSlot_;
		lastSlot_ = myallocator.lastSlot_;
		freeSlots_ = myallocator.freeSlot_;
	}
}
template<typename T>
MyAllocator<T>::~MyAllocator() noexcept
{
	slot_pointer_ curr = currentBlock_;
	while (curr != nullptr)
	{
		slot_pointer_ prev = curr->next;
		operator delete (reinterpret_cast<void*>(curr));
		curr = prev;
	}
}
template<typename T>
inline typename MyAllocator<T>::pointer MyAllocator<T>::address(reference x) const noexcept
{
	return &x;
}
template <typename T>
inline typename MyAllocator<T>::const_pointer MyAllocator<T>::address(const_reference x) const noexcept
{
	return &x;
}
template<typename T>
void MyAllocator<T>::allocateBlock()
{
	data_pointer_ newBlock = reinterpret_cast<data_pointer_>(operator new(BlockSize));
	reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
	currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
	data_pointer_ body = newBlock + sizeof(slot_pointer_);
	size_type bodyPadding = padPointer(body, alignof(slot_type_));
	currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
	lastSlot_ = reinterpret_cast<slot_pointer_>(newBlock + BlockSize - sizeof(slot_type_) + 1);
}
template<typename T>
inline typename MyAllocator<T>::pointer MyAllocator<T>::allocate(size_type n, const_pointer hint)
{
	if (freeSlots_ != nullptr)
	{
		pointer result = reinterpret_cast<pointer>(freeSlots_);
		freeSlots_ = freeSlots_->next;
		return result;
	}
	else
	{
		if (currentSlot_ >= lastSlot_)
			allocateBlock();
		return reinterpret_cast<pointer>(currentSlot_++);
	}
}
template<typename T>
inline void MyAllocator<T>::deallocate(pointer p, size_type n)
{
	if (p != nullptr)
	{
		reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
		freeSlots_ = reinterpret_cast<slot_pointer_>(p);
	}
}
template<typename T>
inline typename MyAllocator<T>::size_type MyAllocator<T>::max_size() const noexcept
{
	size_type maxBlocks = -1 / BlockSize;
	return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_)*maxBlocks;
}
template<typename T>
template<class U, class... Args>
inline void MyAllocator<T>::construct(U* p, Args&&... args)
{
	new (p) U(std::forward<Args>(args)...);
}
template<typename T>
template<class U>
inline void MyAllocator<T>::destroy(U* p)
{
	p->~U();
}
template <typename T>
template <class... Args>
inline typename MyAllocator<T>::pointer MyAllocator<T>::newElement(Args&&... args)
{
	pointer result = allocate();
	construct<value_type>(result, std::forward<Args>(args)...);
	return result;
}
template <typename T>
inline void MyAllocator<T>::deleteElement(pointer p)
{
	if (p != nullptr)
	{
		p->~value_type();
		deallocate(p);
	}
}
#endif // !MY_ALLOCATOR_H