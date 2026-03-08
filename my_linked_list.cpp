#include <memory>
template <typename T>
class allocator {
public:
    using value_type = T;

    T* allocate(size_t n) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }
    void deallocate(T* ptr, size_t) {
        ::operator delete(ptr);
    }
};

template <class T, class U>
bool operator==(allocator<T> const&, allocator<U> const&) noexcept {
    return true;
}

template <typename T, typename Alloc = allocator<T>>
class my_list;

template <typename T>
class my_list_iterator;

template <typename T>
using iterator = my_list_iterator<T>;

template <typename T>
class Node {
public:
    T val;
    Node* next;
    Node* prev;
    Node() : val(T()), next(nullptr), prev(nullptr) {}
    Node(const T& val) : val(val), next(nullptr), prev(nullptr) {}
    friend class my_list<T>;
    friend class my_list_iterator<T>;
};

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

template <typename T>
class my_list_iterator {
    Node<T>* current;

public:
    my_list_iterator(Node<T>* node) : current(node) {}
    T& operator*() {
        return current->val;
    }

    my_list_iterator& operator++() {
        if (current) current = current->next;
        return *this;
    }
    my_list_iterator& operator--() {
        if (current) current = current->prev;
        return *this;
    }

    my_list_iterator operator++(int) {
        my_list_iterator tmp = *this;
        ++(*this);
        return tmp;
    }
    my_list_iterator operator--(int) {
        my_list_iterator tmp = *this;
        --(*this);
        return tmp;
    }

    friend class my_list<T>;
    bool operator==(const my_list_iterator& other) const = default;
};

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

template <typename T, typename Alloc>
class my_list {
    using NodeAllocator = typename std::allocator_traits<Alloc>::template rebind_alloc<Node<T>>;
    using NodeAllocatorTraits = std::allocator_traits<NodeAllocator>;
    NodeAllocator allocator;

public:
    Node<T>* head = nullptr;
    Node<T>* tail = nullptr;
    size_t _size = 0;

    my_list() = default;
    my_list(const T& val);
    my_list(const my_list<T>& other);
    my_list<T>& operator=(my_list<T> other);

    ~my_list();

    void swap(my_list<T>& other);

    void push_front(const T& val);
    void push_back(const T& val);
    void pop_front();
    void pop_back();

    my_list_iterator<T> insert(const my_list_iterator<T>& pos, const T& val);
    my_list_iterator<T> insert(const my_list_iterator<T>& pos, size_t n, const T& val);

    my_list_iterator<T> erase(const my_list_iterator<T>& pos);

    my_list_iterator<T> erase(my_list_iterator<T> first, my_list_iterator<T> last);

    void splice(const my_list_iterator<T>& pos, my_list& list);
    void splice(const my_list_iterator<T>& pos, my_list& list, const my_list_iterator<T>& i);
    void splice(const my_list_iterator<T>& pos, my_list& list, const my_list_iterator<T>& first,
                const my_list_iterator<T>& last);

    void remove(const T& val);

    template <typename Predicate>
    void remove_if(Predicate predicate);

    void unique();

    void reverse();

    void clear() noexcept;

    my_list_iterator<T> begin() {
        return my_list_iterator<T>(head);
    }
    my_list_iterator<T> end() {
        return my_list_iterator<T>(nullptr);
    }
};

template <typename T, typename Alloc>
my_list<T, Alloc>::my_list(const my_list<T>& other)
    : head(nullptr), tail(nullptr), _size(other._size) {
    if (other.head) {
        head = NodeAllocatorTraits::allocate(allocator, 1);
        NodeAllocatorTraits::construct(allocator, head, other.head->val);
        Node<T>* tmp = other.head->next;
        Node<T>* current = head;
        while (tmp) {
            current->next = NodeAllocatorTraits::allocate(allocator, 1);
            NodeAllocatorTraits::construct(allocator, current->next, tmp->val);
            current->next->prev = current;
            current = current->next;
            tmp = tmp->next;
        }
        tail = current;
    }
}

template <typename T, typename Alloc>
my_list<T>& my_list<T, Alloc>::operator=(my_list<T> other) {
    swap(other);
    return *this;
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::swap(my_list<T>& other) {
    std::swap(head, other.head);
    std::swap(tail, other.tail);
    std::swap(_size, other._size);
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::push_front(const T& val) {
    Node<T>* tmp = NodeAllocatorTraits::allocate(allocator, 1);
    NodeAllocatorTraits::construct(allocator, tmp, val);
    ++_size;
    if (!head) {
        head = tail = tmp;
        return;
    }
    tmp->next = head;
    head->prev = tmp;
    head = tmp;
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::push_back(const T& val) {
    Node<T>* tmp = NodeAllocatorTraits::allocate(allocator, 1);
    NodeAllocatorTraits::construct(allocator, tmp, val);
    ++_size;
    if (!head) {
        head = tail = tmp;
        return;
    }
    tmp->prev = tail;
    tail->next = tmp;
    tail = tmp;
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::pop_front() {
    if (!head) return;
    Node<T>* tmp = head;
    head = head->next;
    if (head)
        head->prev = nullptr;
    else
        tail = nullptr;
    NodeAllocatorTraits::destroy(allocator, tmp);
    NodeAllocatorTraits::deallocate(allocator, tmp, 1);
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::pop_back() {
    if (!tail) return;
    Node<T>* tmp = tail;
    if (head == tail)
        head = tail = nullptr;
    else {
        tail = tail->prev;
        tail->next = nullptr;
    }
    NodeAllocatorTraits::destroy(allocator, tmp);
    NodeAllocatorTraits::deallocate(allocator, tmp, 1);
}

template <typename T, typename Alloc>
my_list_iterator<T> my_list<T, Alloc>::insert(const my_list_iterator<T>& pos, const T& val) {
    Node<T>* tmp = NodeAllocatorTraits::allocate(allocator, 1);
    NodeAllocatorTraits::construct(allocator, tmp, val);
    ++_size;

    if (!head) {
        tail = head = tmp;
        return my_list_iterator<T>(tmp);
    }
    if (!pos.current) {
        tmp->prev = tail;
        tail->next = tmp;
        tail = tmp;
        return my_list_iterator<T>(tmp);
    }

    tmp->next = pos.current;
    tmp->prev = pos.current->prev;

    if (pos.current->prev)
        pos.current->prev->next = tmp;
    else
        head = tmp;

    pos.current->prev = tmp;

    return my_list_iterator<T>(tmp);
}

template <typename T, typename Alloc>
my_list_iterator<T> my_list<T, Alloc>::insert(const my_list_iterator<T>& pos, size_t n,
                                              const T& val) {
    Node<T>* first = NodeAllocatorTraits::allocate(allocator, 1);
    NodeAllocatorTraits::construct(allocator, first, val);

    Node<T>* last = first;

    for (size_t i = 1; i < n; ++i) {
        Node<T>* node = NodeAllocatorTraits::allocate(allocator, 1);
        NodeAllocatorTraits::construct(allocator, node, val);
        last->next = node;
        node->prev = last;
        last = node;
    }

    if (!head) {
        // Empty list
        head = tail = first;
    } else if (!pos.current) {
        // Insert at end()
        first->prev = tail;
        tail->next = first;
        tail = last;
    } else {
        // Insert before pos.current
        Node<T>* before = pos.current->prev;
        first->prev = before;
        last->next = pos.current;

        pos.current->prev = last;
        if (before) {
            before->next = first;
        } else {
            head = first;
        }
    }

    return my_list_iterator<T>(first);
}

template <typename T, typename Alloc>
my_list_iterator<T> my_list<T, Alloc>::erase(const my_list_iterator<T>& pos) {
    if (!pos.current) {
        return my_list_iterator<T>(nullptr);
    }
    --_size;
    if (!pos.current->prev) {
        if (!head->next) {
            NodeAllocatorTraits::destroy(allocator, pos.current);
            NodeAllocatorTraits::deallocate(allocator, pos.current, 1);

            head = tail = nullptr;
            return my_list_iterator<T>(nullptr);
        }

        Node<T>* it = pos.current->next;
        head = head->next;
        head->prev = nullptr;
        NodeAllocatorTraits::destroy(allocator, pos.current);
        NodeAllocatorTraits::deallocate(allocator, pos.current, 1);

        return my_list_iterator<T>(it);
    }
    Node<T>* it = pos.current->next;

    if (pos.current->next) {
        pos.current->prev->next = pos.current->next;
        pos.current->next->prev = pos.current->prev;
    } else {
        tail = tail->prev;
        tail->next = nullptr;
        it = nullptr;
    }
    NodeAllocatorTraits::destroy(allocator, pos.current);
    NodeAllocatorTraits::deallocate(allocator, pos.current, 1);
    return my_list_iterator<T>(it);
}

template <typename T, typename Alloc>
my_list_iterator<T> my_list<T, Alloc>::erase(my_list_iterator<T> first, my_list_iterator<T> last) {
    if (!first.current) {
        return my_list_iterator<T>(nullptr);
    }
    --_size;
    if (!first.current->prev) {
        if (!head->next) {
            NodeAllocatorTraits::destroy(allocator, first.current);
            NodeAllocatorTraits::deallocate(allocator, first.current, 1);

            head = tail = nullptr;
            return my_list_iterator<T>(nullptr);
        }

        Node<T>* it = first.current;
        while (first != last) {
            it = first.current;
            head = head->next;
            head->prev = nullptr;
            ++first;
            NodeAllocatorTraits::destroy(allocator, it.current);
            NodeAllocatorTraits::deallocate(allocator, it.current, 1);
        }

        return my_list_iterator<T>(first);
    }
    Node<T>* it = first.current;
    while (first != last) {
        it = first.current;

        if (first.current->next) {
            first.current->prev->next = first.current->next;
            first.current->next->prev = first.current->prev;
        } else {
            tail = tail->prev;
            tail->next = nullptr;
            it = nullptr;
        }
        ++first;
        NodeAllocatorTraits::destroy(allocator, it.current);
        NodeAllocatorTraits::deallocate(allocator, it.current, 1);
    }
    return my_list_iterator<T>(first);
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::splice(const my_list_iterator<T>& pos, my_list& list) {
    if (!pos.current || !list.head) return;

    _size += list._size;
    list._size = 0;

    if (!pos.current->prev) {
        if (!head->next) tail = head;
        head = list.head;
    } else {
        list.head->prev = pos.current->prev;
        pos.current->prev->next = list.head;
    }

    if (!list.head->next) {
        list.head->next = pos.current;
        pos.current->prev = list.head;
    } else {
        list.tail->next = pos.current;
        pos.current->prev = list.tail;
    }

    list.head = list.tail = nullptr;
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::splice(const my_list_iterator<T>& pos, my_list& list,
                               const my_list_iterator<T>& i) {
    if (!pos.current || !i.current) {
        return;
    }
    --_size;

    if (list.head == i.current) {
        if (!list.head->next) {
            list.head = nullptr;
        } else {
            list.head = list.head->next;
            list.head->prev = nullptr;
        }
    } else {
        i.current->prev->next = i.current->next;
        i.current->next->prev = i.current->prev;
    }

    i.current->next = pos.current;

    if (!pos.current->prev) {
        pos.current->prev = i.current;
        i.current->prev = nullptr;
        head = i.current;
    } else {
        pos.current->prev->next = i.current;
        pos.current->prev = i.current;
        i.current->prev = pos.current->prev;
    }
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::splice(const my_list_iterator<T>& pos, my_list& list,
                               const my_list_iterator<T>& first, const my_list_iterator<T>& last) {
    if (!pos.current || !first.current || !last.current) {
        return;
    }
    --_size;

    Node<T>* tmp_last = last.current->prev;
    if (list.head == first.current) {
        if (!list.head->next) {
            list.head = nullptr;
        } else {
            list.head = last.current;
            last.current->prev = nullptr;
        }
    } else {
        first.current->prev->next = last.current;
        last.current->prev = first.current->prev;
    }

    tmp_last->next = pos.current;

    if (!pos.current->prev) {
        pos.current->prev = tmp_last;
        first.current->prev = nullptr;
        head = first.current;
    } else {
        pos.current->prev->next = first.current;
        pos.current->prev = tmp_last;
        first.current->prev = pos.current->prev;
    }
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::remove(const T& val) {
    if (!head) return;

    Node<T>* tmp;
    Node<T>* tmp_to_del = head;

    if (head->val == val) {
        head = head->next;
        head->prev = nullptr;
        NodeAllocatorTraits::destroy(allocator, tmp_to_del);
        NodeAllocatorTraits::deallocate(allocator, tmp_to_del, 1);

        tmp_to_del = nullptr;
    }

    tmp = head;
    tmp_to_del = head;

    while (tmp->next) {
        if (tmp->val == val) {
            tmp->next->prev = tmp->prev;
            tmp->prev->next = tmp->next;

            tmp_to_del = tmp;
            tmp = tmp->next;
            NodeAllocatorTraits::destroy(allocator, tmp_to_del);
            NodeAllocatorTraits::deallocate(allocator, tmp_to_del, 1);

            tmp_to_del = nullptr;
        } else {
            tmp = tmp->next;
            tmp_to_del = tmp;
        }
    }

    if (tmp->val == val) {
        tmp->prev->next = nullptr;
        tail = tmp->prev;
        NodeAllocatorTraits::destroy(allocator, tmp_to_del);
        NodeAllocatorTraits::deallocate(allocator, tmp_to_del, 1);

        tmp_to_del = tmp = nullptr;
    } else {
        tmp->next = nullptr;
        tail = tmp;
    }
}

template <typename T, typename Alloc>
template <typename Predicate>
void my_list<T, Alloc>::remove_if(Predicate predicate) {
    if (!head) return;

    Node<T>* tmp;
    Node<T>* tmp_to_del = head;

    if (predicate(head->val)) {
        head = head->next;
        head->prev = nullptr;
        NodeAllocatorTraits::destroy(allocator, tmp_to_del);
        NodeAllocatorTraits::deallocate(allocator, tmp_to_del, 1);

        tmp_to_del = nullptr;
    }

    tmp = head;
    tmp_to_del = head;

    while (tmp->next) {
        if (predicate(tmp->val)) {
            tmp->next->prev = tmp->prev;
            tmp->prev->next = tmp->next;

            tmp_to_del = tmp;
            tmp = tmp->next;

            NodeAllocatorTraits::destroy(allocator, tmp_to_del);
            NodeAllocatorTraits::deallocate(allocator, tmp_to_del, 1);

            tmp_to_del = nullptr;
        } else {
            tmp = tmp->next;
            tmp_to_del = tmp;
        }
    }
    if (predicate(tmp->val)) {
        tmp->prev->next = nullptr;
        NodeAllocatorTraits::destroy(allocator, tmp_to_del);
        NodeAllocatorTraits::deallocate(allocator, tmp_to_del, 1);

        tmp_to_del = nullptr;
    } else {
        tmp->next = nullptr;
        tail = tmp;
    }
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::unique() {
    if (!head->next) return;

    Node<T>*tmp = head, *tmp_to_del;

    while (tmp->next) {
        if (tmp->val == tmp->next->val) {
            tmp_to_del = tmp->next;
            tmp->next = tmp->next->next;
            tmp->next->prev = tmp;
            NodeAllocatorTraits::destroy(allocator, tmp_to_del);
            NodeAllocatorTraits::deallocate(allocator, tmp_to_del, 1);

            tmp_to_del = nullptr;
        } else {
            tmp = tmp->next;
        }
    }
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::reverse() {
    if (!head->next) return;

    // head implementation

    // Node<T>* current = head,* tmp_prev = nullptr,* tmp_next =  nullptr;
    // head = tail;
    // tail = current;
    // int i = 0;
    // while (current)
    // {
    //     tmp_prev = current->prev;
    //     tmp_next = current->next;
    //     current->next = tmp_prev;
    //     current->prev = tmp_next;
    //     //current = current->prev;
    //     current = tmp_next;

    // }

    // tail implementation
    Node<T>*current = tail, *tmp_prev = nullptr, *tmp_next = nullptr;
    tail = head;
    head = current;
    while (current) {
        tmp_next = current->next;
        tmp_prev = current->prev;
        current->next = tmp_prev;
        current->prev = tmp_next;

        current = tmp_prev;
    }
}

template <typename T, typename Alloc>
void my_list<T, Alloc>::clear() noexcept {
    Node<T>* tmp;
    while (head) {
        tmp = head;
        head = head->next;
        NodeAllocatorTraits::destroy(allocator, tmp);
        NodeAllocatorTraits::deallocate(allocator, tmp, 1);
    }
    _size = 0;
    head = nullptr;
}

template <typename T, typename Alloc>
my_list<T, Alloc>::~my_list() {
    clear();
}

int main() {}
