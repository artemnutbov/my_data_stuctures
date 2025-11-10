#include <iostream>
#define SIZE_OF_BUCKET  32
#define DEFAULT_SIZE  2
#define SIZE_SCALE  2


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
    size_t size_ = 0;
    size_t arr_size_ = 0;
public:
    using value_type = T;

    my_deque() = default;
    void push_back(const T& val);

    ~my_deque();
};

template <typename T>
void my_deque<T>::push_back(const T& val) {
    ++size_;
    if(!arr_) {
        arr_ = new T*[DEFAULT_SIZE];
        arr_size_ = DEFAULT_SIZE;
        front_.i = back_.i = 1;
        front_.j = back_.j = 0;
        for(size_t i = 0; i < arr_size_;++i){
            arr_[i] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET*sizeof(T)]);
        }
            
        new(arr_[back_.i]) T(val); // check if this throw and how it. next time !!!
        return;
    }
    if(++back_.j == SIZE_OF_BUCKET ) {
        back_.j = 0;
        if(++back_.i == arr_size_){
            // reallocate
            T** newarr = new T*[arr_size_* SIZE_SCALE];
            size_t half_size = arr_size_/2;
            front_.i += half_size;
            back_.i += half_size;
            size_t index = 0;
            for(;index < half_size;++index) {
                newarr[index] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET*sizeof(T)]);
            }
            for(int i = 0;index < arr_size_ + half_size;++index, ++i) {
                newarr[index] = arr_[i]; 
            }
            arr_size_ *= SIZE_SCALE;
            for(;index < arr_size_ ;++index) {
                newarr[index] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET*sizeof(T)]);
            }
            delete[] reinterpret_cast<char*>(arr_);
            arr_ = newarr; 
        }
    }
    new(arr_[back_.i]) T(val);
    
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
    for(int i = 0; i < 1000;++i) {
        d.push_back(15);
    }
    
}