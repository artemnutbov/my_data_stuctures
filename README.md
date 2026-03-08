# My C++ Data Structures

This repo is a collection of my custom data structures that I code while studying C++. It's not a replacement  STL, I just want to know how some containers work under the hood. Some of the data structures are a bit 
messy(the early ones, specifically). Additionally, for `my_string`, `my_forward_list` and `my_linked_list` you need C++ 20.


 To build these, I use three different memory mechanics. For `my_string`, `my_forward_list` and 2 hash tables(collision handling via open addressing and separate chaining), I use raw `new` and `delete`. For `my_linked_list` use `std::allocator_traits`. 
  And for `my_deque` use placement `new` and explicit destructor calls(`->~T()`) with `try. . .catch` blocks to provide the Strong Exception Guarantee.

*  `my_string`: It includes implementation of the Knuth-Morris-Pratt(KMP) algorithm for substring searching. And spaceship operator(<=>) implementation.

*  `my_forward_list`: A single linked list. Has O(N \log N) in-place Merge Sort. Instead of inefficiently swapping values, the sorting algorithm rewires the underlying node pointers.

*  `my_linked_list`: A double linked list. Has bidirectional iterator and constant-time splicing.

*  `hash_table_OA`(Open Addressing): A flat array. I implemented a state system(0 = empty, 1 = occupied, 2 = deleted) for correct search after deleting. Also, done Rule of Five.

*  `hash_table_SC`(Separate Chaining): An array of pointers to linked nodes. Use `std::exchange` in the move constructor. Also, done Rule of Five.

*  `my_deque`: Implemented using a 2D pointer array to fixed-size buckets. Achieves O(1) time complexity for `push_front`, `pop_front`, `push_back` and  `pop_back`.
  Overloaded `operator[]` calculates the bucket and offset in O(1) time. `base_iterator` uses a template parameter(`<bool IsConst>`)  for  `std::conditional_t` to generate  `iterator` and `const_iterator`
  types without duplicating code. Making `my_deque` fully compatible with `<algorithm>`. Also, done Rule of Five.

> "What I cannot create, I do not understand." 

> -Richard Feynman
