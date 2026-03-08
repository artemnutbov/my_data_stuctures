#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, typename T>
struct Bucket {
    Key key = Key();
    T value = T();
    int8_t state = 0;
};

template <typename Key, typename T>
class hash_table_OA {
public:
    void initialize_index(size_t i, const Key& key, const T& value);
    void delete_index(size_t i);

    size_t size;
    size_t bucket_count;
    Bucket<Key, T>* array;

    size_t get_index(const Key& key);

    hash_table_OA();
    void insert(const Key& key, const T& value);
    void rehash();

    hash_table_OA(const hash_table_OA& other);

    hash_table_OA<Key, T>& operator=(hash_table_OA<Key, T> other);

    hash_table_OA<Key, T>& operator=(hash_table_OA<Key, T>&& other);

    hash_table_OA(hash_table_OA&& other);

    void swap(hash_table_OA<Key, T>& other);

    void erase(const Key& key);

    void clear();

    T& operator[](const Key& key);
    const T& operator[](const Key& key) const;

    ~hash_table_OA();
};

template <typename Key, typename T>
hash_table_OA<Key, T>::hash_table_OA()
    : size(0), bucket_count(8), array(new Bucket<Key, T>[bucket_count] {}) {}

template <typename Key, typename T>
hash_table_OA<Key, T>::hash_table_OA(const hash_table_OA& other)
    : size(other.size),
      bucket_count(other.bucket_count),
      array(new Bucket<Key, T>[other.bucket_count] {}) {
    for (size_t i = 0; i < bucket_count; ++i) array[i] = other.array[i];
}

template <typename Key, typename T>
hash_table_OA<Key, T>::hash_table_OA(hash_table_OA&& other) {
    array = other.array;
    bucket_count = other.bucket_count;
    size = other.size;
    other.array = nullptr;
    other.bucket_count = other.size = 0;
}

template <typename Key, typename T>
hash_table_OA<Key, T>& hash_table_OA<Key, T>::operator=(hash_table_OA<Key, T> other) {
    swap(other);
    return *this;
}

template <typename Key, typename T>
hash_table_OA<Key, T>& hash_table_OA<Key, T>::operator=(hash_table_OA<Key, T>&& other) {
    if (&other == this) return *this;

    delete[] array;

    array = other.array;
    bucket_count = other.bucket_count;
    size = other.size;
    other.array = nullptr;
    other.bucket_count = other.size = 0;

    return *this;
}

template <typename Key, typename T>
void hash_table_OA<Key, T>::swap(hash_table_OA<Key, T>& other) {
    using std::swap;
    swap(array, other.array);
    swap(bucket_count, other.bucket_count);
    swap(size, other.size);
}

template <typename Key, typename T>
size_t hash_table_OA<Key, T>::get_index(const Key& key) {
    std::hash<Key> h;
    return (h(key) & (bucket_count - 1));
}

template <typename Key, typename T>
void hash_table_OA<Key, T>::initialize_index(size_t i, const Key& key, const T& value) {
    array[i].value = value;
    array[i].key = key;
    array[i].state = 1;
    ++size;
}

template <typename Key, typename T>
void hash_table_OA<Key, T>::delete_index(size_t i) {
    array[i].key = Key();
    array[i].value = T();

    array[i].state = 2;
    --size;
}

template <typename Key, typename T>
void hash_table_OA<Key, T>::insert(const Key& key, const T& value) {
    if (size * 1.25 > bucket_count) rehash();
    size_t index = get_index(key);

    if (0 == array[index].state) {
        initialize_index(index, key, value);
        return;
    }

    for (size_t i = index; i < bucket_count; ++i) {
        if (0 == array[i].state) {
            initialize_index(i, key, value);
            return;
        }
    }

    for (int i = index; i >= 0; --i) {
        if (0 == array[i].state) {
            initialize_index(i, key, value);
            return;
        }
    }
}

template <typename Key, typename T>
void hash_table_OA<Key, T>::rehash() {
    Bucket<Key, T>* tmp = array;

    size_t n = bucket_count;
    bucket_count *= 2;
    array = new Bucket<Key, T>[bucket_count];

    size = 0;
    for (size_t i = 0; i < n; ++i) {
        if (1 == tmp[i].state) {
            insert(tmp[i].key, tmp[i].value);
        }
    }
    delete[] tmp;
}

template <typename Key, typename T>
T& hash_table_OA<Key, T>::operator[](const Key& key) {
    if (size * 1.25 > bucket_count) rehash();
    size_t index = get_index(key);

    if (0 == array[index].state) {
        array[index].key = key;
        array[index].state = 1;
        ++size;
        return array[index].value;
    }

    if (array[index].key == key && 2 != array[index].state) return array[index].value;

    int step_index_forw = -11;
    int step_index_back = -11;

    size_t i = index;
    for (; i < bucket_count; ++i) {
        if (array[i].key == key) return array[i].value;
        if (0 == array[i].state) {
            step_index_forw = i - index;
            break;
        }
    }
    int j = index;
    for (; j >= 0; --j) {
        if (array[j].key == key) return array[j].value;
        if (0 == array[j].state) {
            step_index_back = abs(j - index);
            break;
        }
    }
    if (-11 == step_index_back && -11 == step_index_forw) {
        throw std::runtime_error("hash table is full");
    }
    size_t pos;
    if (step_index_back == -11 || step_index_forw <= step_index_back)
        pos = i;
    else
        pos = j;

    array[pos].key = key;
    array[pos].state = 1;
    ++size;

    return array[pos].value;
}

template <typename Key, typename T>
void hash_table_OA<Key, T>::clear() {
    for (size_t i = 0; i < bucket_count; ++i) {
        if (1 == array[i].state) delete_index(i);
    }
}

template <typename Key, typename T>
void hash_table_OA<Key, T>::erase(const Key& key) {
    size_t index = get_index(key);
    if (array[index].key == key) {
        delete_index(index);
        return;
    }
    for (size_t i = index; i < bucket_count; ++i) {
        if (array[i].key == key) {
            delete_index(i);
            return;
        }
        if (0 == array[i].state) break;
    }

    for (int i = index; i >= 0; --i) {
        if (array[i].key == key) {
            delete_index(i);
            return;
        }
        if (0 == array[i].state) break;
    }
}

template <typename Key, typename T>
hash_table_OA<Key, T>::~hash_table_OA() {
    delete[] array;
}

template <typename Key, typename T>
void swap(hash_table_OA<Key, T>& a, hash_table_OA<Key, T>& b) {
    a.swap(b);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

template <typename Key, typename T>
struct Node {
    Key key = Key();
    T value = T();
    Node* next = nullptr;
    //    Node() = default;
};

template <typename Key, typename T>
class hash_table_SC {
public:
    size_t size;
    size_t bucket_count;
    Node<Key, T>** buckets;
    hash_table_SC();
    hash_table_SC(const hash_table_SC<Key, T>& other);
    hash_table_SC(hash_table_SC<Key, T>&& other) noexcept;

    ~hash_table_SC();
    hash_table_SC<Key, T>& operator=(const hash_table_SC<Key, T>& other);
    hash_table_SC<Key, T>& operator=(hash_table_SC<Key, T>&& other) noexcept;

    void swap(hash_table_SC<Key, T>& other) noexcept;
    size_t get_index(const Key& key);
    void insert(const Key& key, const T& value);
    void remove_bucket(Node<Key, T>* head);
    T& operator[](const Key& key);
    void erase(const Key& key);
    void rehash();
};

template <typename Key, typename T>
hash_table_SC<Key, T>::hash_table_SC()
    : size(0), bucket_count(8), buckets(new Node<Key, T>* [bucket_count] {}) {}

template <typename Key, typename T>
hash_table_SC<Key, T>::hash_table_SC(const hash_table_SC<Key, T>& other)
    : size(other.size),
      bucket_count(other.bucket_count),
      buckets(new Node<Key, T>* [bucket_count] {}) {
    for (size_t i = 0; i < bucket_count; ++i) {
        if (other.buckets[i]) {
            Node<Key, T>* tmp = other.buckets[i];
            buckets[i] = new Node<Key, T>{tmp->key, tmp->value, nullptr};
            tmp = tmp->next;
            while (tmp) {
                Node<Key, T>* current = new Node<Key, T>{tmp->key, tmp->value, nullptr};
                current->next = buckets[i];
                buckets[i] = current;
                tmp = tmp->next;
            }
        }
    }
}

template <typename Key, typename T>
hash_table_SC<Key, T>::hash_table_SC(hash_table_SC<Key, T>&& other) noexcept
    : size(std::exchange(other.size, 0)),
      bucket_count(std::exchange(other.bucket_count, 0)),
      buckets(std::exchange(other.buckets, nullptr)) {}

template <typename Key, typename T>
hash_table_SC<Key, T>& hash_table_SC<Key, T>::operator=(const hash_table_SC& other) {
    if (this != &other) {
        hash_table_SC tmp(other);
        swap(tmp);
    }
    return *this;
}

template <typename Key, typename T>
hash_table_SC<Key, T>& hash_table_SC<Key, T>::operator=(hash_table_SC<Key, T>&& other) noexcept {
    if (this == &other) return *this;
    this->~hash_table_SC();
    buckets = std::exchange(other.buckets, nullptr);
    size = std::exchange(other.size, 0);
    bucket_count = std::exchange(other.bucket_count, 0);
    return *this;
}

template <typename Key, typename T>
void hash_table_SC<Key, T>::swap(hash_table_SC<Key, T>& other) noexcept {
    using std::swap;
    swap(buckets, other.buckets);
    swap(bucket_count, other.bucket_count);
    swap(size, other.size);
}

template <typename Key, typename T>
hash_table_SC<Key, T>::~hash_table_SC() {
    for (size_t i = 0; i < bucket_count; ++i) {
        if (buckets[i]) remove_bucket(buckets[i]);
    }
    delete[] buckets;
}

template <typename Key, typename T>
size_t hash_table_SC<Key, T>::get_index(const Key& key) {
    std::hash<Key> h;
    return h(key) & (bucket_count - 1);
}

template <typename Key, typename T>
void hash_table_SC<Key, T>::insert(const Key& key, const T& value) {
    size_t index = get_index(key);
    ++size;
    if (size * 1.25 > bucket_count) rehash();

    if (!buckets[index])
        buckets[index] = new Node<Key, T>{key, value, nullptr};
    else {
        Node<Key, T>* tmp = new Node<Key, T>{key, value, nullptr};
        tmp->next = buckets[index];
        buckets[index] = tmp;
    }
}

template <typename Key, typename T>
void hash_table_SC<Key, T>::remove_bucket(Node<Key, T>* head) {
    while (head) {
        Node<Key, T>* tmp = head;
        head = tmp->next;
        delete tmp;
    }
    head = nullptr;
}

template <typename Key, typename T>
T& hash_table_SC<Key, T>::operator[](const Key& key) {
    size_t index = get_index(key);
    if (size * 1.25 > bucket_count) rehash();

    if (!buckets[index]) {
        ++size;
        buckets[index] = new Node<Key, T>{key, T(), nullptr};
    }

    if (buckets[index]->key == key)
        return buckets[index]->value;
    else {
        Node<Key, T>* tmp = buckets[index];
        while (tmp) {
            if (tmp->key == key) return tmp->value;
            tmp = tmp->next;
        }
        ++size;
        tmp = new Node<Key, T>{key, T(), nullptr};
        tmp->next = buckets[index];
        buckets[index] = tmp;
        return tmp->value;
    }
}

template <typename Key, typename T>
void hash_table_SC<Key, T>::erase(const Key& key) {
    size_t index = get_index(key);
    if (!buckets[index]) return;

    if (key == buckets[index]->key) {
        --size;
        if (buckets[index]->next) {
            Node<Key, T>* tmp = buckets[index]->next;
            delete buckets[index];
            buckets[index] = tmp;
            return;
        }
        delete buckets[index];
        buckets[index] = nullptr;
        return;
    }

    Node<Key, T>* current = buckets[index];
    while (current->next) {
        if (key == current->next->key) {
            --size;
            if (current->next->next) {
                Node<Key, T>* tmp = current->next->next;
                delete current->next;
                current->next = tmp;
                return;
            }
            delete current->next;
            current->next = nullptr;
            return;
        }
        current = current->next;
    }
}

template <typename Key, typename T>
void hash_table_SC<Key, T>::rehash() {
    Node<Key, T>** tmp = buckets;
    size_t n = bucket_count;
    bucket_count *= 2;
    buckets = new Node<Key, T>* [bucket_count] {};
    size = 0.0;
    for (size_t i = 0; i < n; ++i) {
        if (tmp[i]) {
            Node<Key, T>* current = tmp[i];
            while (current) {
                insert(current->key, current->value);
                current = current->next;
            }
        }
    }
    for (size_t i = 0; i < n; ++i) {
        if (tmp[i]) remove_bucket(tmp[i]);
    }
    delete[] tmp;
}

template <typename Key, typename T>
void swap(hash_table_SC<Key, T>& lhs, hash_table_SC<Key, T>& rhs) noexcept {
    lhs.swap(rhs);
}

int main() {}