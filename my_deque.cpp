#include <iostream>
#define SIZE_OF_BUCKET = 32;


struct Index
{
    int i;
    int j;
    Index() = default;
    Index(int i,int j):i(i),j(j) {}
};

template <typename T> 
class my_deque {
    T** _arr = nullptr;
    Index _front;
    Index _back;
    size_t _size;
    my_deque() = default;
};


int main() {

    
}