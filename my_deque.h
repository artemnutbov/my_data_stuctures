#include <iostream>

struct Index {
    size_t i;
    size_t j;
    Index() = default;
    Index(size_t i, size_t j) : i(i), j(j) {}
};

template <typename T, typename Alloc = std::allocator<T>>
class my_deque {
public:
    using value_type = T;
    using allocator_type = Alloc;
    using node_traits = std::allocator_traits<Alloc>;

    using array_allocator_type = typename node_traits::template rebind_alloc<T*>;
    using array_traits = std::allocator_traits<array_allocator_type>;

private:
    [[no_unique_address]] allocator_type node_alloc_;
    [[no_unique_address]] array_allocator_type array_alloc_;

    T** arr_ = nullptr;
    size_t size_ = 0;
    size_t arr_size_ = 0;
    Index front_;
    Index back_;

    static constexpr size_t SIZE_OF_BUCKET = 32;
    static constexpr size_t SIZE_SCALE = 2;
    static constexpr size_t DEFAULT_SIZE = 2;

    template <bool IsConst>
    class base_iterator {
    public:
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;

        using pointer_arr_type = std::conditional_t<IsConst, const T**, T**>;
        using reference_arr_type = std::conditional_t<IsConst, const T*&, T*&>;

        using value_type = T;

    private:
        pointer_arr_type parr_;
        Index position;
        base_iterator(T** parr_, Index position) : parr_(parr_), position(position) {}
        friend class my_deque;

    public:
        base_iterator(const base_iterator&) = default;
        base_iterator& operator=(const base_iterator&) = default;

        reference_type operator*() const {
            return parr_[position.i][position.j];
        }
        pointer_type operator->() const {
            return parr_[position.i] + position.j;
        }

        base_iterator& operator++() {
            if (++position.j == SIZE_OF_BUCKET) {
                position.j = 0;
                ++position.i;
            }
            return *this;
        }

        base_iterator operator++(int) {
            base_iterator copy = *this;
            ++(*this);
            return copy;
        }

        base_iterator& operator--() {
            if (position.j == 0) {
                position.j = SIZE_OF_BUCKET - 1;
                --position.i;
            } else
                --position.j;
            return *this;
        }

        base_iterator operator--(int) {
            base_iterator copy = *this;
            --(*this);
            return copy;
        }
        bool operator==(const base_iterator& other) {
            return (position.i == other.position.i) && (position.j == other.position.j);
        }

        bool operator!=(const base_iterator& other) {
            return !(*this == other);
        }

        operator base_iterator<true>() const {
            return parr_[position.i] + position.j;
        }
    };

public:
    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using size_type = typename node_traits::size_type;
    using difference_type = typename node_traits::difference_type;
    using pointer = typename node_traits::pointer;
    using const_pointer = typename node_traits::const_pointer;
    using reference = value_type&;
    using const_reference = const value_type&;

    iterator begin() {
        return {arr_, front_};
    }

    iterator end() {
        if (back_.j == SIZE_OF_BUCKET - 1) return {arr_, {back_.i + 1, 0}};
        return {arr_, {back_.i, back_.j + 1}};
    }

    const_iterator begin() const {
        return {arr_, front_};
    }

    const_iterator end() const {
        if (back_.j == SIZE_OF_BUCKET - 1) return {arr_, {back_.i + 1, 0}};
        return {arr_, {back_.i, back_.j + 1}};
    }

    const_iterator cbegin() const {
        return {arr_, front_};
    }

    const_iterator cend() const {
        if (back_.j == SIZE_OF_BUCKET - 1) return {arr_, {back_.i + 1, 0}};
        return {arr_, {back_.i, back_.j + 1}};
    }

    my_deque() = default;

    explicit my_deque(const allocator_type& alloc) noexcept
        : node_alloc_(alloc),
          array_alloc_(alloc),
          arr_(nullptr),
          size_(0),
          arr_size_(0),
          front_({0, 0}),
          back_({0, 0}) {}

    my_deque(const my_deque& other, const allocator_type& alloc)
        : node_alloc_(alloc),
          array_alloc_(alloc),
          arr_(nullptr),
          size_(other.size_),
          arr_size_(other.arr_size_),
          front_(other.front_),
          back_(other.back_) {
        deep_copy(other);
    }
    my_deque(my_deque&& other, const allocator_type& alloc) noexcept
        : node_alloc_(alloc),
          array_alloc_(alloc),
          arr_(nullptr),
          size_(0),
          arr_size_(0),
          front_({0, 0}),
          back_({0, 0}) {
        if (node_alloc_ == other.node_alloc_) {
            steal_memory(std::move(other));
        } else {
            for (auto it = other.begin(); it != other.end(); ++it) {
                push_back(std::move(*it));
            }
            other.clear();
        }
    }

    my_deque(const my_deque& other)
        : node_alloc_(node_traits::select_on_container_copy_construction(other.node_alloc_)),
          array_alloc_(array_traits::select_on_container_copy_construction(other.array_alloc_)),
          arr_(nullptr),
          size_(other.size_),
          arr_size_(other.arr_size_),
          front_(other.front_),
          back_(other.back_) {
        deep_copy(other);
    }

    my_deque(my_deque&& other) noexcept
        : node_alloc_(std::move(other.node_alloc_)),
          array_alloc_(std::move(other.array_alloc_)),
          arr_(other.arr_),
          size_(other.size_),
          arr_size_(other.arr_size_),
          front_(other.front_),
          back_(other.back_) {
        other.arr_ = nullptr;
        other.size_ = 0;
        other.arr_size_ = 0;
        other.front_ = {0, 0};
        other.back_ = {0, 0};
    }
    allocator_type get_allocator() const noexcept {
        return node_alloc_;
    }
    my_deque& operator=(const my_deque& other) {
        if (this == &other) {
            return *this;
        }
        if constexpr (node_traits::propagate_on_container_copy_assignment::value) {
            if (node_alloc_ != other.node_alloc_) {
                clear_and_deallocate();
            }
            node_alloc_ = other.node_alloc_;
            array_alloc_ = other.array_alloc_;
        }

        my_deque tmp(other, node_alloc_);
        swap(tmp);
        return *this;
    }

    void swap(my_deque& other) {
        if constexpr (node_traits::propagate_on_container_swap::value) {
            std::swap(node_alloc_, other.node_alloc_);
            std::swap(array_alloc_, other.array_alloc_);
        }

        std::swap(arr_, other.arr_);
        std::swap(front_, other.front_);
        std::swap(back_, other.back_);
        std::swap(size_, other.size_);
        std::swap(arr_size_, other.arr_size_);
    }

    my_deque& operator=(my_deque&& other) noexcept {
        if (this == &other) return *this;

        constexpr bool pocma = node_traits::propagate_on_container_move_assignment::value;

        if constexpr (pocma) {
            clear_and_deallocate();
            node_alloc_ = std::move(other.node_alloc_);
            array_alloc_ = std::move(other.array_alloc_);
            steal_memory(std::move(other));
        } else if (node_alloc_ == other.node_alloc_) {
            clear_and_deallocate();
            steal_memory(std::move(other));
        } else {
            clear();
            for (auto it = other.begin(); it != other.end(); ++it) {
                push_back(std::move(*it));
            }

            other.clear();
        }

        return *this;
    }

    void push_back(const T& val) {
        if (!arr_) {
            T** new_arr = nullptr;
            size_t index = 0;
            try {
                new_arr = array_traits::allocate(array_alloc_, DEFAULT_SIZE);

                for (; index < DEFAULT_SIZE; ++index) {
                    new_arr[index] = node_traits::allocate(node_alloc_, SIZE_OF_BUCKET);
                }
                node_traits::construct(node_alloc_, new_arr[1], val);
            } catch (...) {
                for (size_t i = 0; i < index; ++i) {
                    node_traits::deallocate(node_alloc_, new_arr[i], SIZE_OF_BUCKET);
                }
                array_traits::deallocate(array_alloc_, new_arr, DEFAULT_SIZE);

                throw;
            }

            arr_ = new_arr;
            front_.i = back_.i = 1;
            front_.j = back_.j = 0;
            arr_size_ = DEFAULT_SIZE;
            ++size_;
            return;
        }

        Index old_back = back_;
        if (++back_.j == SIZE_OF_BUCKET) {
            if (++back_.i == arr_size_) {
                // reallocate
                const size_t new_size = arr_size_ * SIZE_SCALE;
                T** new_arr = nullptr;
                size_t half = arr_size_ / 2;

                try {
                    new_arr = array_traits::allocate(array_alloc_, new_size);
                    for (size_t i = 0; i < new_size; ++i) {
                        array_traits::construct(array_alloc_, new_arr + i, nullptr);
                    }

                    for (size_t i = 0; i < half; ++i) {
                        new_arr[i] = node_traits::allocate(node_alloc_, SIZE_OF_BUCKET);
                    }

                    for (size_t i = 0; i < arr_size_; ++i) {
                        new_arr[i + half] = arr_[i];
                    }

                    for (size_t i = half + arr_size_; i < new_size; ++i) {
                        new_arr[i] = node_traits::allocate(node_alloc_, SIZE_OF_BUCKET);
                    }

                }

                catch (...) {
                    if (new_arr) {
                        for (size_t i = 0; i < half; ++i) {
                            if (new_arr[i] != nullptr) {
                                node_traits::deallocate(node_alloc_, new_arr[i], SIZE_OF_BUCKET);
                            }
                        }
                        for (size_t i = half + arr_size_; i < new_size; ++i) {
                            if (new_arr[i] != nullptr) {
                                node_traits::deallocate(node_alloc_, new_arr[i], SIZE_OF_BUCKET);
                            }
                        }
                        array_traits::deallocate(array_alloc_, new_arr, new_size);
                    }
                    throw;
                }

                array_traits::deallocate(array_alloc_, arr_, arr_size_);

                arr_ = new_arr;
                front_.i += half;
                back_.i += half;
                arr_size_ = new_size;
                old_back.i += half;
            }
            back_.j = 0;
        }
        try {
            node_traits::construct(node_alloc_, arr_[back_.i] + back_.j, val);
            ++size_;
        } catch (...) {
            back_ = old_back;
            throw;
        }
    }

    void push_front(const T& val) {
        if (!arr_) {
            T** new_arr = nullptr;
            size_t index = 0;
            try {
                new_arr = array_traits::allocate(array_alloc_, DEFAULT_SIZE);

                for (; index < DEFAULT_SIZE; ++index) {
                    new_arr[index] = node_traits::allocate(node_alloc_, SIZE_OF_BUCKET);
                }
                node_traits::construct(node_alloc_, new_arr[1], val);

            } catch (...) {
                for (size_t i = 0; i < index; ++i) {
                    node_traits::deallocate(node_alloc_, new_arr[i], SIZE_OF_BUCKET);
                }
                array_traits::deallocate(array_alloc_, new_arr, DEFAULT_SIZE);
                throw;
            }

            arr_ = new_arr;
            front_.i = back_.i = 1;
            front_.j = back_.j = 0;
            arr_size_ = DEFAULT_SIZE;
            ++size_;
            return;
        }
        Index old_front = front_;
        if (front_.j == 0) {
            front_.j = SIZE_OF_BUCKET - 1;

            if (front_.i == 0) {
                // reallocate
                const size_t new_size = arr_size_ * SIZE_SCALE;
                T** new_arr = nullptr;
                size_t half = arr_size_ / 2;

                try {
                    new_arr = array_traits::allocate(array_alloc_, new_size);
                    for (size_t i = 0; i < new_size; ++i) {
                        array_traits::construct(array_alloc_, new_arr + i, nullptr);
                    }
                    for (size_t i = 0; i < half; ++i) {
                        new_arr[i] = node_traits::allocate(node_alloc_, SIZE_OF_BUCKET);
                    }

                    for (size_t i = 0; i < arr_size_; ++i) {
                        new_arr[i + half] = arr_[i];
                    }

                    for (size_t i = half + arr_size_; i < new_size; ++i) {
                        new_arr[i] = node_traits::allocate(node_alloc_, SIZE_OF_BUCKET);
                    }
                }

                catch (...) {
                    if (new_arr) {
                        for (size_t i = 0; i < half; ++i) {
                            if (new_arr[i] != nullptr) {
                                node_traits::deallocate(node_alloc_, new_arr[i], SIZE_OF_BUCKET);
                            }
                        }
                        for (size_t i = half + arr_size_; i < new_size; ++i) {
                            if (new_arr[i] != nullptr) {
                                node_traits::deallocate(node_alloc_, new_arr[i], SIZE_OF_BUCKET);
                            }
                        }
                        array_traits::deallocate(array_alloc_, new_arr, new_size);
                    }
                    throw;
                }

                array_traits::deallocate(array_alloc_, arr_, arr_size_);
                arr_ = new_arr;
                front_.i += half;
                back_.i += half;
                arr_size_ = new_size;
                old_front.i += half;
            }
            --front_.i;
        } else {
            --front_.j;
        }
        try {
            node_traits::construct(node_alloc_, arr_[front_.i] + front_.j, val);
            ++size_;
        } catch (...) {
            front_ = old_front;
            throw;
        }
    }

    void pop_front() {
        if (size_ == 0) return;
        --size_;
        if (!arr_) return;
        node_traits::destroy(node_alloc_, arr_[front_.i] + front_.j);
        if (++front_.j == SIZE_OF_BUCKET) {
            front_.j = 0;
            ++front_.i;
        }
    }

    void pop_back() {
        --size_;
        if (!arr_) return;
        node_traits::destroy(node_alloc_, arr_[back_.i] + back_.j);
        if (back_.j == 0) {
            back_.j = SIZE_OF_BUCKET - 1;
            --back_.i;
        } else
            --back_.j;
    }

    T& operator[](size_t index) {
        size_t remainder = index % SIZE_OF_BUCKET;

        Index tmp(front_.i + index / SIZE_OF_BUCKET, front_.j + remainder);
        if (front_.j + remainder >= SIZE_OF_BUCKET) {
            ++tmp.i;
            tmp.j -= SIZE_OF_BUCKET;
        }
        return arr_[tmp.i][tmp.j];
    }

    ~my_deque() {
        clear_and_deallocate();
    }

private:
    void clear() noexcept {
        if (arr_ && size_ > 0) {
            if (front_.i == back_.i) {
                for (size_t i = front_.j; i <= back_.j; ++i) {
                    node_traits::destroy(node_alloc_, arr_[front_.i] + i);
                }
            } else {
                for (size_t i = front_.j; i < SIZE_OF_BUCKET; ++i) {
                    node_traits::destroy(node_alloc_, arr_[front_.i] + i);
                }
                for (size_t i = front_.i + 1; i < back_.i; ++i) {
                    for (size_t j = 0; j < SIZE_OF_BUCKET; ++j) {
                        node_traits::destroy(node_alloc_, arr_[i] + j);
                    }
                }

                for (size_t i = 0; i <= back_.j; ++i) {
                    node_traits::destroy(node_alloc_, arr_[back_.i] + i);
                }
            }
            size_ = 0;
            size_t middle_bucket = arr_size_ / 2;
            front_.i = back_.i = middle_bucket;
            front_.j = back_.j = 0;
        }
    }
    void deep_copy(const my_deque& other) {
        if (!other.arr_ || other.arr_size_ == 0) {
            arr_ = nullptr;
            return;
        }
        T** new_arr = array_traits::allocate(array_alloc_, other.arr_size_);

        size_t index_start = 0;
        try {
            for (; index_start < arr_size_; ++index_start) {
                new_arr[index_start] = node_traits::allocate(node_alloc_, SIZE_OF_BUCKET);
            }
        } catch (...) {
            for (size_t i = 0; i < index_start; ++i) {
                node_traits::deallocate(node_alloc_, new_arr[i], SIZE_OF_BUCKET);
            }
            array_traits::deallocate(array_alloc_, new_arr, other.arr_size_);

            throw;
        }
        index_start = front_.j;

        if (front_.i == back_.i) {
            try {
                for (; index_start <= back_.j; ++index_start) {
                    node_traits::construct(node_alloc_, new_arr[front_.i] + index_start,
                                           other.arr_[front_.i][index_start]);
                }
            } catch (...) {
                for (size_t i = front_.j; i < index_start; ++i) {
                    node_traits::destroy(node_alloc_, new_arr[front_.i] + i);
                }
                for (size_t i = 0; i < arr_size_; ++i) {
                    node_traits::deallocate(node_alloc_, new_arr[i], SIZE_OF_BUCKET);
                }
                array_traits::deallocate(array_alloc_, new_arr, other.arr_size_);
                throw;
            }
        } else {
            size_t index_end = 0;
            size_t middle_index_i = front_.i + 1;

            try {
                for (; index_start < SIZE_OF_BUCKET; ++index_start) {
                    node_traits::construct(node_alloc_, new_arr[front_.i] + index_start,
                                           other.arr_[front_.i][index_start]);
                }
                for (; middle_index_i < back_.i; ++middle_index_i) {
                    size_t constructed = 0;
                    try {
                        for (; constructed < SIZE_OF_BUCKET; ++constructed) {
                            node_traits::construct(node_alloc_,
                                                   new_arr[middle_index_i] + constructed,
                                                   other.arr_[middle_index_i][constructed]);
                        }
                    } catch (...) {
                        for (size_t j = 0; j < constructed; ++j) {
                            node_traits::destroy(node_alloc_, new_arr[middle_index_i] + j);
                        }
                        throw;
                    }
                }
                for (; index_end <= back_.j; ++index_end) {
                    node_traits::construct(node_alloc_, new_arr[back_.i] + index_end,
                                           other.arr_[back_.i][index_end]);
                }

            } catch (...) {
                for (size_t i = front_.j; i < index_start; ++i) {
                    node_traits::destroy(node_alloc_, new_arr[front_.i] + i);
                }

                for (size_t i = front_.i + 1; i < middle_index_i; ++i) {
                    for (size_t j = 0; j < SIZE_OF_BUCKET; ++j) {
                        node_traits::destroy(node_alloc_, new_arr[i] + j);
                    }
                }

                for (size_t i = 0; i < index_end; ++i) {
                    node_traits::destroy(node_alloc_, new_arr[back_.i] + i);
                }

                for (size_t i = 0; i < arr_size_; ++i) {
                    node_traits::deallocate(node_alloc_, new_arr[i], SIZE_OF_BUCKET);
                }
                array_traits::deallocate(array_alloc_, new_arr, other.arr_size_);
                throw;
            }
        }
        arr_ = new_arr;
    }
    void steal_memory(my_deque&& other) {
        arr_ = other.arr_;
        size_ = other.size_;
        arr_size_ = other.arr_size_;
        front_ = other.front_;
        back_ = other.back_;

        other.arr_ = nullptr;
        other.size_ = 0;
        other.arr_size_ = 0;
        other.front_ = {0, 0};
        other.back_ = {0, 0};
    }
    void clear_and_deallocate() {
        if (arr_) {
            clear();
            for (size_t i = 0; i < arr_size_; ++i) {
                node_traits::deallocate(node_alloc_, arr_[i], SIZE_OF_BUCKET);
            }

            array_traits::deallocate(array_alloc_, arr_, arr_size_);
            arr_ = nullptr;
            size_ = 0;
            arr_size_ = 0;
            front_ = {0, 0};
            back_ = {0, 0};
        }
    }
};
