#include <iostream>
#define SIZE_OF_BUCKET  32
#define DEFAULT_SIZE  2

struct Index
{
    size_t i;
    size_t j;
    Index() = default;
    Index(size_t i,size_t j):i(i),j(j) {}
};

template <typename T> 
class my_deque {
    T** arr_ = nullptr;
    Index front_;
    Index back_;
    size_t size_;
    size_t arr_size_;
public:
    using value_type = T;

    my_deque() = default;
    void push_back(const T& val);

    ~my_deque();
};

template <typename T>
void my_deque<T>::push_back(const T& val) {
    if(!arr_) {
        arr_ = new T*[DEFAULT_SIZE];
        arr_size_ = DEFAULT_SIZE;
        front_.i = back_.i = 1;
        front_.j = back_.j = 0;
        for(size_t i = 0; i < arr_size_;++i){
            arr_[i] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET*sizeof(T)]);
        }
            
        new(arr_[back_.i]) T(val); // check for throw next time

    }
}

template <typename T>
my_deque<T>::~my_deque() {
    if(arr_) {
        for(size_t i = front_.i;i < back_.i;++i) {
            for(size_t j = front_.j;j < back_.j;++j) {
                (arr_[i] + j)->~T();
            }
        }

        for(size_t i = 0;i < arr_size_;++i){
            delete[] reinterpret_cast<char*>(arr_[i]);
        }

        delete[] arr_;
    }
}
int main() {
    my_deque<int> d;
    d.push_back(3);

    
}