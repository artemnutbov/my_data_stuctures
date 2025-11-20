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
        base_iterator(T**parr_,Index position):parr_(parr_),position(position) {}
        friend class my_deque<T>;
    public:
        base_iterator(const base_iterator&) = default;
        base_iterator& operator=(const base_iterator&) = default;
        
        reference_type operator*() const { return parr_[position.i][position.j];}
        pointer_type operator->() const { return parr_[position.i] + position.j; }
        
        base_iterator& operator++() {
            if(++position.j == SIZE_OF_BUCKET) {
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
            if(position.j == 0) {
                position.j = SIZE_OF_BUCKET - 1;
                --position.i;
            } 
            else
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
            return  parr_[position.i] + position.j;
        }
    };
public:
    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    
    
    iterator begin() {
        return {arr_,front_};
    }

    iterator end() {
        if(back_.j == SIZE_OF_BUCKET - 1)
            return {arr_ ,{back_.i+1,0}};
        return {arr_,{back_.i,back_.j+1}};
    
    }

    
    const_iterator begin() const {
        return {arr_,front_};
    }

    const_iterator end() const {
        if(back_.j == SIZE_OF_BUCKET - 1)
            return {arr_ ,{back_.i+1,0}};
        return {arr_,{back_.i,back_.j+1}};
    }

    const_iterator cbegin() const {
        return {arr_ ,front_};
    }

    const_iterator cend() const{
        if(back_.j == SIZE_OF_BUCKET - 1)
            return {arr_ ,{back_.i+1,0}};
        return {arr_,{back_.i,back_.j+1}};
    }

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
my_deque<T>::my_deque(const my_deque& other):arr_(nullptr),front_(other.front_),back_(other.back_),size_(other.size_),arr_size_(other.arr_size_) {
    T** new_arr = new T*[other.arr_size_];
    size_t index_start = 0;
    try{
        for(; index_start < arr_size_;++index_start){
            new_arr[index_start] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET*sizeof(T)]);
        }
    } catch(...) {
        for(size_t i = 0;i < index_start;++i){
            delete[] reinterpret_cast<char*>(new_arr[i]);
        }
        delete[] new_arr;
        throw;
    }
    index_start = front_.j;
    
    if(front_.i == back_.i) {
        try{  
            for(;index_start <= back_.j; ++index_start)
                new(new_arr[front_.i] + index_start) T(other.arr_[front_.i][index_start]);
        } catch(...) {
            for(size_t i = front_.j;i < index_start;++i){
                (new_arr[front_.i] + i)->~T();
            }
            for(size_t i = 0; i < arr_size_;++i){
                delete[] reinterpret_cast<char*>(new_arr[i]);
            }
            delete[] new_arr;
            throw;
        }
    }
    else {
        size_t index_end = 0;
        size_t middle_index_i = front_.i + 1;

        try{
            for(;index_start < SIZE_OF_BUCKET; ++index_start)
                new(new_arr[front_.i] + index_start) T(other.arr_[front_.i][index_start]);


            for(; middle_index_i < back_.i; ++middle_index_i) {
                size_t constructed = 0;
                try {
                    for(; constructed < SIZE_OF_BUCKET; ++constructed)
                        new(new_arr[middle_index_i] + constructed) T(other.arr_[middle_index_i][constructed]);
                    
                } catch(...) {
                    for(size_t j = 0; j < constructed; ++j)
                        (new_arr[middle_index_i] + j)->~T();
                    throw;
                }
            }
            size_t counter = 0;
            for(;index_end <= back_.j; ++index_end){
                new(new_arr[back_.i] + index_end) T(other.arr_[back_.i][index_end]);
                ++counter;

            }


        } catch(...) {
            for(size_t i = front_.j;i < index_start;++i){
                (new_arr[front_.i] + i)->~T();
            }

            for(size_t i = front_.i + 1;i < middle_index_i;++i) {
                for(size_t j = 0;j < SIZE_OF_BUCKET  ;++j) {
                    (new_arr[i] + j)->~T();
                }
            }

            for(size_t i = 0;i < index_end;++i){
                (new_arr[back_.i] + i)->~T();
            }

            for(size_t i = 0; i < arr_size_;++i){
                delete[] reinterpret_cast<char*>(new_arr[i]);
            }
            delete[] new_arr;
            throw;
        }
    }
    arr_ = new_arr;
    
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
    if(!arr_) {
        T** new_arr = nullptr;
        size_t index = 0;
        try{
            new_arr = new T*[DEFAULT_SIZE];

            for(; index < DEFAULT_SIZE;++index){
                new_arr[index] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET*sizeof(T)]);
            }
            new(new_arr[1]) T(val); 

        } catch(...) {
            for(size_t i = 0;i < index;++i){
                delete[] reinterpret_cast<char*>(new_arr[i]);
            }
            delete[] new_arr;
            throw;
        }

        arr_ = new_arr;
        front_.i = back_.i = 1;
        front_.j = back_.j = 0;
        arr_size_ = DEFAULT_SIZE;
        ++size_;     
        return;
    }

    if(++back_.j == SIZE_OF_BUCKET ) {
        
        if(++back_.i == arr_size_){
            // reallocate
            const size_t new_size = arr_size_ * SIZE_SCALE;
            T** new_arr = nullptr;
            size_t half = arr_size_/2;

            try{
                new_arr =  new T*[new_size]{nullptr};
                
                for (size_t i = 0; i < half; ++i) {
                    new_arr[i] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET * sizeof(T)]);
                }

                for (size_t i = 0; i < arr_size_; ++i) {
                    new_arr[i + half] = arr_[i];
                }

                for (size_t i = half + arr_size_; i < new_size; ++i) {
                    new_arr[i] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET * sizeof(T)]);
                }
                
            } 
            
            catch(...) {
                
                if (new_arr) {
                    for (size_t i = 0; i < half; ++i)
                        delete[] reinterpret_cast<char*>(new_arr[i]);

                    for (size_t i = half + arr_size_; i < new_size; ++i)
                        delete[] reinterpret_cast<char*>(new_arr[i]);

                    delete[] new_arr;
                }
                throw;
            }


            delete[] arr_;
            
            arr_ = new_arr; 
            front_.i += half;
            back_.i += half;
            arr_size_ = new_size;
        }
        back_.j = 0;
    }
    new(arr_[back_.i] + back_.j) T(val);
}



template <typename T>
void my_deque<T>::push_front(const T& val) {
    if(!arr_) {
        T** new_arr = nullptr;
        size_t index = 0;
        try{
            new_arr = new T*[DEFAULT_SIZE];

            for(; index < DEFAULT_SIZE;++index){
                new_arr[index] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET*sizeof(T)]);
            }
            new(new_arr[1]) T(val); 

        } catch(...) {
            for(size_t i = 0;i < index;++i){
                delete[] reinterpret_cast<char*>(new_arr[i]);
            }
            delete[] new_arr;
            throw;
        }

        arr_ = new_arr;
        front_.i = back_.i = 1;
        front_.j = back_.j = 0;
        arr_size_ = DEFAULT_SIZE;
        ++size_;     
        return;
    }

    if(front_.j == 0 ) {
        front_.j = SIZE_OF_BUCKET - 1;

        if(front_.i == 0){
            // reallocate
            const size_t new_size = arr_size_ * SIZE_SCALE;
            T** new_arr = nullptr;
            size_t half = arr_size_/2;

            try{
                new_arr =  new T*[new_size]{nullptr};

                for (size_t i = 0; i < half; ++i) {
                    new_arr[i] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET * sizeof(T)]);
                }

                for (size_t i = 0; i < arr_size_; ++i) {
                    new_arr[i + half] = arr_[i];
                }

                for (size_t i = half + arr_size_; i < new_size; ++i) {
                    new_arr[i] = reinterpret_cast<T*>(new char[SIZE_OF_BUCKET * sizeof(T)]);
                }
            } 
            
            catch(...) {
                
                if (new_arr) {
                    for (size_t i = 0; i < half; ++i)
                        delete[] reinterpret_cast<char*>(new_arr[i]);

                    for (size_t i = half + arr_size_; i < new_size; ++i)
                        delete[] reinterpret_cast<char*>(new_arr[i]);

                    delete[] new_arr;
                }
                throw;
            }

            delete[] arr_;
            arr_ = new_arr;
            front_.i += half;
            back_.i += half;
            arr_size_ = new_size; 
        }
        --front_.i;
        new(arr_[front_.i] + front_.j) T(val);
    }
    else {
        --front_.j;
        new(arr_[front_.i] + front_.j) T(val);
    }
    ++size_;

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

    my_deque<int> d;
    for(int i = 0; i < 40;++i) {
        d.push_back(i);
    }
    for(auto i : d) {
        std::cout << i << '\t';
    }
    // for(auto i = d.begin();i != d.end();i++) {
    //     *i = 5;
    // }
    // for(auto i : d) {
    //     std::cout << i << '\t';
    // }
    std::cout << '\n';
    const my_deque<int>::iterator it = d.begin();
    //std::cout << *it << *(it++) << '\n';
    std::cout << *it << '\n';
    
    
    
    // try{
    //     for(int i = 0; i < 4;++i) {
    //         std::vector<int> v{i,i*i};
    //         //d.push_front(v);

    //         d.push_back(v);
    //     }
        
    //     my_deque<std::vector<int>> dd;
    //     dd = d;
    //     for(int i = 0; i < 4;++i) {
    //         dd[i][0] = 12;
    //     }
    //     for(int i = 0; i < 4;++i) {
    //         std::cout << dd[i][0] << ' ' << dd[i][1] << ' '; 
            
    //     }


    //     for(int i = 0; i < 4;++i) {
    //         std::vector<int> v{i,2,3,4,5};
    //         dd.push_front(v);
    //         dd.push_back(v);
    //     }
    //     my_deque<std::vector<int>> ddd = dd;

    // }
    // catch(...){
    //     std::cerr << "\ncaught!!\n";
    //     return 0;
    // }
    std::cout << "\n\n\n\n++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";    
}