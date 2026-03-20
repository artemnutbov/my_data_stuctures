#include <cstddef>
#include <memory>

template <typename T, size_t MaxSize>
struct StackAllocator;

template <size_t MaxSize>
class StackStorage {
    template <typename U, size_t M>
    friend struct StackAllocator;

    std::byte* start_ptr_;
    std::byte* top_ptr_;

    [[nodiscard]] void* allocate(size_t bytes, size_t alignment) {
        if (bytes > MaxSize) throw std::bad_alloc();

        size_t remaining_space = MaxSize - static_cast<size_t>(top_ptr_ - start_ptr_);
        void* ptr = static_cast<void*>(top_ptr_ + 1);

        if (remaining_space < 1) throw std::bad_alloc();

        remaining_space -= 1;

        // std::align modify remaining_space
        void* result = std::align(alignment, bytes, ptr, remaining_space);

        if (!result) throw std::bad_alloc();

        std::byte* casted_ptr = static_cast<std::byte*>(ptr);
        size_t shift = casted_ptr - top_ptr_;

        // prevent uint8_t overflow
        if (shift > 255) throw std::bad_alloc();

        *(casted_ptr - 1) = static_cast<std::byte>(shift);

        top_ptr_ = casted_ptr + bytes;
        return result;
    }

    void deallocate(void* ptr, size_t bytes) noexcept {
        std::byte* casted_ptr = static_cast<std::byte*>(ptr);
        if (casted_ptr + bytes == top_ptr_)
            top_ptr_ = casted_ptr - std::to_integer<size_t>(*(casted_ptr - 1));
    }

public:
    StackStorage(std::byte* start_ptr) noexcept : start_ptr_(start_ptr), top_ptr_(start_ptr) {}
};
template <typename T, size_t MaxSize>
class StackAllocator {
    // let see private storage all other templates of this allocator
    template <typename U, size_t M>
    friend class StackAllocator;

    StackStorage<MaxSize>* storage_;

public:
    // for allocator_traits
    using value_type = T;
    using is_always_equal = std::false_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    StackAllocator(StackStorage<MaxSize>* storage) noexcept : storage_(storage) {}

    // rebind constructor
    template <typename U>
    StackAllocator(const StackAllocator<U, MaxSize>& other) noexcept : storage_(other.storage_) {}

    [[nodiscard]] T* allocate(size_t count) {
        if (count == 0) return nullptr;
        if (count > MaxSize / sizeof(T)) throw std::bad_alloc();
        void* result = storage_->allocate(count * sizeof(T), alignof(T));
        return static_cast<T*>(result);
    }
    void deallocate(T* ptr, size_t count) noexcept {
        if (!ptr) return;
        storage_->deallocate(ptr, count * sizeof(T));
    }

    template <typename U>
    bool operator==(const StackAllocator<U, MaxSize>& rhs) const noexcept {
        return storage_ == rhs.storage_;
    }

    template <typename U>
    bool operator!=(const StackAllocator<U, MaxSize>& rhs) const noexcept {
        return !(*this == rhs);
    }

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, MaxSize>;
    };
};
