#include <iostream>

template <typename T>
class my_forward_list;

template <typename T>
class my_forward_list_iterator;

template <typename T>
using iterator = my_forward_list_iterator<T>;

template <typename T>
class Node {
    T val;
    Node* next;
    Node() : val(T()), next(nullptr) {}
    Node(const T& val) : val(val), next(nullptr) {}
    friend class my_forward_list<T>;
    friend class my_forward_list_iterator<T>;
};

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

template <typename T>
class my_forward_list_iterator {
    Node<T>* current;

public:
    my_forward_list_iterator(Node<T>* node) : current(node) {}
    T& operator*() {
        return current->val;
    }

    my_forward_list_iterator& operator++() {
        if (current) current = current->next;
        return *this;
    }
    friend class my_forward_list<T>;
    bool operator==(const my_forward_list_iterator& other) const = default;
};

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

template <typename T>
class my_forward_list {
    Node<T>* head = nullptr;

    template <typename Compare>
    Node<T>* merge(Node<T>* l1, Node<T>* l2, Compare comp);

    template <typename Compare>
    Node<T>* merge_sort(Node<T>* head, Compare comp);

public:
    my_forward_list() = default;
    my_forward_list(const T& val);
    my_forward_list(size_t n, const T& val);
    my_forward_list(const my_forward_list& other);

    my_forward_list<T>& operator=(my_forward_list<T> other);
    void swap(my_forward_list<T>& other) noexcept;

    void assign(size_t n, const T& val);

    void push_front(const T& val);
    void pop_front();

    iterator<T> insert_after(iterator<T>, const T& val);
    iterator<T> insert_after(iterator<T>, int n, const T& val);

    template <typename InputIterator>
    iterator<T> insert_after(iterator<T> pos, InputIterator first, InputIterator last);

    iterator<T> erase_after(iterator<T> pos);
    iterator<T> erase_after(iterator<T> pos, iterator<T> last);

    T& front();
    bool empty() const noexcept;
    void clear() noexcept;
    void resize(size_t n);

    void splice_after(iterator<T> pos, my_forward_list& fwdlst);
    void splice_after(iterator<T> pos, my_forward_list& fwdlst, iterator<T> i);
    void splice_after(iterator<T> pos, my_forward_list& fwdlst, iterator<T> first,
                      iterator<T> last);

    void remove(const T& val);

    template <typename Predicate>
    void remove_if(Predicate pred);

    void unique();

    template <class BinaryPredicate>
    void unique(BinaryPredicate binary_pred);

    void reverse() noexcept;

    template <typename Compare>
    void sort(Compare comp);
    void sort();

    template <class Compare>
    void merge(my_forward_list<T>& fwdlst, Compare comp);

    void merge(my_forward_list<T>& fwdlst);

    my_forward_list_iterator<T> begin() {
        return my_forward_list_iterator<T>(head);
    }
    my_forward_list_iterator<T> end() {
        return my_forward_list_iterator<T>(nullptr);
    }

    T& operator[](size_t pos);

    ~my_forward_list();
};

//----------------------------------------------------------------------------------------------

template <typename T>
my_forward_list<T>::my_forward_list(const T& val) : head(new Node<T>(val)) {}

//----------------------------------------------------------------------------------------------

template <typename T>
my_forward_list<T>::~my_forward_list() {
    clear();
}

//----------------------------------------------------------------------------------------------

template <typename T>
my_forward_list<T>::my_forward_list(size_t n, const T& val) {
    if (0 != n) {
        size_t i = 0;
        head = new Node<T>(val);
        Node<T>* tmp;
        while (i < n - 1) {
            tmp = new Node<T>(val);
            tmp->next = head;
            head = tmp;
            ++i;
        }
    }
}

//----------------------------------------------------------------------------------------------

template <typename T>
my_forward_list<T>::my_forward_list(const my_forward_list<T>& other) {
    head = new Node(other.head->val);
    Node<T>* tmp_other = other.head->next;
    Node<T>* tmp = head;
    while (tmp_other) {
        tmp->next = new Node<T>(tmp_other->val);
        tmp = tmp->next;
        tmp_other = tmp_other->next;
    }
}

//----------------------------------------------------------------------------------------------
template <typename T>
my_forward_list<T>& my_forward_list<T>::operator=(my_forward_list<T> other) {
    swap(other);
    return *this;
}

//----------------------------------------------------------------------------------------------

template <typename T>
void my_forward_list<T>::swap(my_forward_list<T>& other) noexcept {
    std::swap(head, other.head);
}

//----------------------------------------------------------------------------------------------
template <typename T>
void my_forward_list<T>::assign(size_t n, const T& val) {
    clear();
    if (0 == n) return;
    size_t i = 0;
    head = new Node<T>(val);
    Node<T>* tmp;
    while (i < n - 1) {
        tmp = new Node<T>(val);
        tmp->next = head;
        head = tmp;
        ++i;
    }
}

//----------------------------------------------------------------------------------------------

template <typename T>
T& my_forward_list<T>::front() {
    return head->val;
}

//----------------------------------------------------------------------------------------------

template <typename T>
bool my_forward_list<T>::empty() const noexcept {
    return !head;
}

//----------------------------------------------------------------------------------------------

template <typename T>
void my_forward_list<T>::resize(size_t n) {
    if (0 == n) {
        clear();
        return;
    }

    if (!head) {
        head = new Node<T>();
        Node<T>* tmp = head;
        for (size_t i = 1; i < n; ++i) {
            tmp->next = new Node<T>(T());
            tmp = tmp->next;
        }
        return;
    }
    size_t count = 1;
    Node<T>* tmp = head;
    while (tmp->next && count < n) {
        tmp = tmp->next;
        ++count;
    }

    if (count == n) {
        Node<T>* tmp_del = tmp->next;
        tmp->next = nullptr;
        while (tmp_del) {
            Node<T>* next = tmp_del->next;
            delete tmp_del;
            tmp_del = next;
        }
    } else {
        while (count < n) {
            tmp->next = new Node<T>(T());
            tmp = tmp->next;
            ++count;
        }
    }
}

//----------------------------------------------------------------------------------------------

template <typename T>
void my_forward_list<T>::clear() noexcept {
    while (head) {
        Node<T>* tmp = head;
        head = tmp->next;
        delete tmp;
    }
    head = nullptr;
}

//----------------------------------------------------------------------------------------------

template <typename T>
iterator<T> my_forward_list<T>::insert_after(iterator<T> pos, const T& val) {
    if (pos.current) {
        Node<T>* tmp = new Node<T>(val);
        tmp->next = pos.current->next;
        ;
        pos.current->next = tmp;
        return my_forward_list_iterator<T>(tmp);
    }
    return end();
}

//----------------------------------------------------------------------------------------------

template <typename T>
iterator<T> my_forward_list<T>::insert_after(iterator<T> pos, int n, const T& val) {
    if (pos.current) {
        Node<T>* tmp;
        int i = 0;
        while (i < n) {
            tmp = new Node<T>(val);
            tmp->next = pos.current->next;
            ;
            pos.current->next = tmp;
            ++i;
        }
        return my_forward_list_iterator<T>(tmp);
    }
    return end();
}

//----------------------------------------------------------------------------------------------

template <typename T>
template <typename InputIterator>
iterator<T> my_forward_list<T>::insert_after(iterator<T> pos, InputIterator first,
                                             InputIterator last) {
    if (pos.current) {
        Node<T>* tmp;
        while (first != last) {
            tmp = new Node<T>(*first);
            tmp->next = pos.current->next;
            pos.current->next = tmp;
            ++first;
        }
        return my_forward_list_iterator<T>(pos.current->next);
    }
    return end();
}

//----------------------------------------------------------------------------------------------
template <typename T>
iterator<T> my_forward_list<T>::erase_after(iterator<T> pos) {
    if (pos.current) {
        Node<T>* tmp = pos.current->next;
        pos.current->next = tmp->next;
        delete tmp;
        return my_forward_list_iterator<T>(pos.current->next);
    }
    return end();
}

//----------------------------------------------------------------------------------------------
template <typename T>
iterator<T> my_forward_list<T>::erase_after(iterator<T> pos, iterator<T> last) {
    if (pos.current) {
        Node<T>* tmp = pos.current->next;

        while (pos.current->next != last) {
            tmp = pos.current->next;
            pos.current->next = tmp->next;
            delete tmp;
        }
        return my_forward_list_iterator<T>(pos.current->next);
    }
    return end();
}

//----------------------------------------------------------------------------------------------

template <typename T>
void my_forward_list<T>::push_front(const T& val) {
    if (!head) {
        head = new Node<T>(val);
        return;
    }
    Node<T>* tmp = new Node<T>(val);
    tmp->next = head;
    head = tmp;
}

//----------------------------------------------------------------------------------------------

template <typename T>
void my_forward_list<T>::pop_front() {
    if (!head) return;  // don't know if at all this need?

    Node<T>* tmp = head->next;
    delete head;
    head = tmp;
}

//----------------------------------------------------------------------------------------------

template <typename T>
void my_forward_list<T>::splice_after(iterator<T> pos, my_forward_list& fwdlst) {
    if (!pos.current) return;
    Node<T>* tmp = pos.current->next;
    pos.current->next = fwdlst.head;

    Node<T>* last = fwdlst.head;
    while (last->next) {
        last = last->next;
    }

    last->next = tmp;
    fwdlst.head = nullptr;
}

//----------------------------------------------------------------------------------------------

template <typename T>
void my_forward_list<T>::splice_after(iterator<T> pos, my_forward_list& fwdlst, iterator<T> i) {
    if (!pos.current || !i.current) return;
    Node<T>* tmp = pos.current->next;
    pos.current->next = i.current;

    Node<T>* tmp_other = i.current->next;

    i.current->next = tmp;

    if (i.current == fwdlst.head) {
        fwdlst.head = tmp_other;
        return;
    }

    tmp = fwdlst.head;

    while (tmp->next != i.current) {
        tmp = tmp->next;
    }

    tmp->next = tmp_other;
}

template <typename T>
void my_forward_list<T>::splice_after(iterator<T> pos, my_forward_list& fwdlst, iterator<T> first,
                                      iterator<T> last) {
    if (!pos.current || !first.current || !last.current) return;
    Node<T>* tmp_other = pos.current->next;
    pos.current->next = first.current->next;

    Node<T>* tmp = fwdlst.head;
    while (tmp->next != last.current) {
        tmp = tmp->next;
    }
    Node<T>* tmp_last = tmp->next;
    tmp->next = tmp_other;

    tmp = fwdlst.head;
    while (tmp != first.current) {
        tmp = tmp->next;
    }
    tmp->next = tmp_last;
}

template <typename T>
void my_forward_list<T>::remove(const T& val) {
    Node<T>*tmp, *tmp_del;
    if (val == head->val) {
        tmp_del = head;
        head = head->next;
        delete tmp_del;
    }
    tmp = head;
    while (tmp->next) {
        if (val == tmp->next->val) {
            tmp_del = tmp->next;
            tmp->next = tmp->next->next;
            delete tmp_del;
        } else
            tmp = tmp->next;
    }
}

template <typename T>
template <typename Predicate>
void my_forward_list<T>::remove_if(Predicate pred) {
    Node<T>*tmp, *tmp_del;
    if (pred(head->val)) {
        tmp_del = head;
        head = head->next;
        delete tmp_del;
    }
    tmp = head;
    while (tmp->next) {
        if (pred(tmp->next->val)) {
            tmp_del = tmp->next;
            tmp->next = tmp->next->next;
            delete tmp_del;
        } else
            tmp = tmp->next;
    }
}

template <typename T>
void my_forward_list<T>::unique() {
    if (!head) return;
    Node<T>*current = head, *tmp_del;

    while (current->next) {
        if (current->val == current->next->val) {
            tmp_del = current->next->next;
            delete current->next;
            current->next = tmp_del;
        } else {
            current = current->next;
        }
    }
}

template <typename T>
template <class BinaryPredicate>
void my_forward_list<T>::unique(BinaryPredicate binary_pred) {
    if (!head) return;
    Node<T>*current = head, *tmp_del;

    while (current->next) {
        if (current->val == current->next->val || binary_pred(current->val, current->next->val)) {
            tmp_del = current->next->next;
            delete current->next;
            current->next = tmp_del;
        } else {
            current = current->next;
        }
    }
}

template <typename T>
void my_forward_list<T>::reverse() noexcept {
    // // my implementation

    // if(!head) return;
    // Node<T>* current = head , *tmp;
    // while(current->next) {
    //     tmp = current->next;
    //     current->next = tmp->next;
    //     tmp->next = head;
    //     head = tmp;
    // }

    //  standard approach

    Node<T>* prev = nullptr;
    Node<T>* current = head;
    Node<T>* next = nullptr;

    while (current) {          // Traverse through the list
        next = current->next;  // Save the next node
        current->next = prev;  // Reverse the link
        prev = current;        // Move prev forward
        current = next;        // Move current forward
    }
    head = prev;  // Update head to the last node (new front)
}

template <typename T>
template <typename Compare>
void my_forward_list<T>::sort(Compare comp) {
    if (!head || !head->next) return;  // No sorting needed if list is empty or has 1 element

    head = merge_sort(head, comp);
}

template <typename T>
void my_forward_list<T>::sort() {
    if (!head || !head->next) return;  // No sorting needed if list is empty or has 1 element

    head = merge_sort(head, [](T a, T b) { return a < b; });
}

template <typename T>
template <typename Compare>
Node<T>* my_forward_list<T>::merge_sort(Node<T>* head, Compare comp) {
    if (!head || !head->next) return head;

    Node<T>*slow = head, *fast = head->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    Node<T>* mid = slow->next;
    slow->next = nullptr;

    Node<T>* left = merge_sort(head, comp);
    Node<T>* right = merge_sort(mid, comp);

    return merge(left, right, comp);
}

template <typename T>
template <typename Compare>
Node<T>* my_forward_list<T>::merge(Node<T>* l1, Node<T>* l2, Compare comp) {
    if (!l1) return l2;
    if (!l2) return l1;

    Node<T>* head = nullptr;
    if (comp(l1->val, l2->val)) {
        head = l1;
        l1 = l1->next;
    } else {
        head = l2;
        l2 = l2->next;
    }

    Node<T>* tail = head;
    while (l1 && l2) {
        if (comp(l1->val, l2->val)) {
            tail->next = l1;
            l1 = l1->next;
        } else {
            tail->next = l2;
            l2 = l2->next;
        }
        tail = tail->next;
    }

    tail->next = l1 ? l1 : l2;

    return head;
}

template <typename T>
template <class Compare>
void my_forward_list<T>::merge(my_forward_list<T>& fwdlst, Compare comp) {
    if (this == &fwdlst || !fwdlst.head) return;

    if (!head) {  // If the current list is empty just take the other list
        head = fwdlst.head;
        fwdlst.head = nullptr;
        return;
    }

    Node<T>* first = head;
    Node<T>* second = fwdlst.head;

    if (second && comp(second->val, first->val)) {
        std::swap(first, second);
        head = first;
    }

    Node<T>* tail = first;
    first = first->next;

    while (first && second) {
        if (comp(second->val, first->val)) {
            tail->next = second;
            second = second->next;
        } else {
            tail->next = first;
            first = first->next;
        }
        tail = tail->next;
    }

    tail->next = first ? first : second;

    fwdlst.head = nullptr;
}

template <typename T>
void my_forward_list<T>::merge(my_forward_list<T>& fwdlst) {
    if (this == &fwdlst || !fwdlst.head) return;

    if (!head) {
        head = fwdlst.head;
        fwdlst.head = nullptr;
        return;
    }

    Node<T>* first = head;
    Node<T>* second = fwdlst.head;

    if (second && second->val < first->val) {
        std::swap(first, second);
        head = first;
    }

    Node<T>* tail = first;
    first = first->next;

    while (first && second) {
        if (second->val < first->val) {
            tail->next = second;
            second = second->next;
        } else {
            tail->next = first;
            first = first->next;
        }
        tail = tail->next;
    }

    tail->next = first ? first : second;

    fwdlst.head = nullptr;
}

template <typename T>
T& my_forward_list<T>::operator[](size_t pos) {
    size_t i = 0;
    Node<T>* tmp = head;
    while (i < pos) {
        tmp = tmp->next;
        ++i;
    }
    return tmp->val;
}

int main() {}