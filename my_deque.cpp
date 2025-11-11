#include <iostream>
#include <vector>
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
    my_deque(const my_deque& other);
    my_deque& operator=(my_deque other);
    void push_back(const T& val);
    void push_front(const T& val);
    void pop_back();
    void pop_front();
    void swap(my_deque& other);

    T& operator[](size_t index);


    ~my_deque();
};

template <typename T>
my_deque<T>::my_deque(const my_deque& other):arr_(new T*[other.arr_size_]),front_(other.front_),back_(other.back_),size_(other.size_),arr_size_(other.arr_size_) {
    for(size_t i = 0; i < arr_size_;++i){
        arr_[i] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET*sizeof(T)]);
    }

    if(front_.i == back_.i) {
        for(size_t i = front_.j;i <= back_.j; ++i)
            new(arr_[front_.i] + i) T(other.arr_[front_.i][i]);
    }
    else {
        for(size_t i = front_.j;i < SIZE_OF_BUCKET; ++i)
            new(arr_[front_.i] + i) T(other.arr_[front_.i][i]);
                
            
        for(size_t i = front_.i + 1;i < back_.i ;++i) {
            for(size_t j = 0;j < SIZE_OF_BUCKET;++j) {
                new(arr_[i] + j) T(other.arr_[i][j]);
            }
        }

        for(size_t i = 0;i <= back_.j; ++i)
            new(arr_[back_.i] + i) T(other.arr_[back_.i][i]);

    }
    // need to write exception safety
}

template <typename T>
my_deque<T>& my_deque<T>::operator=(my_deque other) {
    swap(other);
    return *this;
}

template <typename T>
void my_deque<T>::swap(my_deque& other) {
    std::swap(arr_,other.arr_);
    std::swap(front_,other.front_);
    std::swap(back_,other.back_);
    std::swap(size_,other.size_);
    std::swap(arr_size_,other.arr_size_);
}



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
    new(arr_[back_.i] + back_.j) T(val);
}



template <typename T>
void my_deque<T>::push_front(const T& val) {
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

    if(front_.j == 0 ) {
        front_.j = SIZE_OF_BUCKET - 1;

        if(front_.i == 0){
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
        --front_.i;
        new(arr_[front_.i] + front_.j) T(val);
    }
    else {
        --front_.j;
        new(arr_[front_.i] + front_.j) T(val);
    }
}

template <typename T>
void my_deque<T>::pop_back() {
    --size_;
    if(!arr_) return;
    (arr_[back_.i] + back_.j)->~T();
    if(back_.j == 0) {
        back_.j = SIZE_OF_BUCKET - 1;
        --back_.i;
    }
    else
        --back_.j;
}


template <typename T>
void my_deque<T>::pop_front() {
    --size_;
    if(!arr_) return;
    (arr_[front_.i] + front_.j)->~T();
    if(++front_.j == SIZE_OF_BUCKET) {
        front_.j = 0;
        ++front_.i;
    }
}

template <typename T>
T& my_deque<T>::operator[](size_t index) {
    size_t remainder = index % SIZE_OF_BUCKET; 
    
    Index tmp(front_.i + index / SIZE_OF_BUCKET, front_.j + remainder);
    if(front_.j + remainder >= SIZE_OF_BUCKET) {
        ++tmp.i;
        tmp.j -= SIZE_OF_BUCKET;
    }
    return arr_[tmp.i][tmp.j];

}


template <typename T>
my_deque<T>::~my_deque() {
    if(arr_) {
        if(front_.i == back_.i) {
            for(size_t i = front_.j;i <= back_.j; ++i)
                (arr_[front_.i] + i)->~T();
        }
        else {
            for(size_t i = front_.j;i < SIZE_OF_BUCKET; ++i)
                (arr_[front_.i] + i)->~T();
            
            for(size_t i = front_.i + 1;i < back_.i ;++i) {
                for(size_t j = 0;j < SIZE_OF_BUCKET;++j) {
                    (arr_[i] + j)->~T();
                }
            }

            for(size_t i = 0;i <= back_.j; ++i)
                (arr_[back_.i] + i)->~T();
        }

        for(size_t i = 0;i < arr_size_;++i){
            delete[] reinterpret_cast<char*>(arr_[i]);
        }

        delete[] arr_;
    }
}

int main() {
    // my_deque<int> d;
    // for(int i = 0; i < 2000;++i) {
    //     d.push_front(i);
    //     d.push_back(i);
    // }
    // for(int i = 0; i < 1000;++i) {
    //     d.pop_back();
    // }
    // for(int i = 0; i < 1000;++i) {
    //     d.pop_front();
    // }

    
    // //my_deque<int> dd(d);
    // for(int i = 0; i < 2000;++i) {
    //     std::cout << d[i] << ' ';
    // }
 
    my_deque<std::vector<int>> d;
    for(int i = 0; i < 400;++i) {
        std::vector<int> v{i,i*i};

        d.push_front(v);
        d.push_back(v);
    }

    my_deque<std::vector<int>> dd;
    dd = d;
    for(int i = 0; i < 800;++i) {
        dd[i][0] = 12;
    }
    for(int i = 0; i < 800;++i) {
        std::cout << dd[i][0] << ' ' << dd[i][1] << ' '; 
        
    }


    // for(int i = 0; i < 400;++i) {
    //     std::vector<int> v{i,2,3,4,5};
    //     dd.push_front(v);
    //     dd.push_back(v);
    // }
    // my_deque<std::vector<int>> ddd = dd;
    
}