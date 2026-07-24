#include <iostream>

template <typename Key, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key>>
class my_set {
    struct BaseNode {
        BaseNode *left = nullptr, *right = nullptr, *parent = nullptr;
        BaseNode() = default;

        bool isOnLeft() {
            return this == parent->left;
        }

        bool isOnRight() {
            return this == parent->right;
        }

        BaseNode *sibling() {
            return (isOnLeft()) ? parent->right : parent->left;
        }
        virtual ~BaseNode() {}
    };

    struct Node : BaseNode {
        const Key key;
        bool is_red;
        Node(const Key &key, bool is_red) : key(key), is_red(is_red) {}
    };

    using value_type = Key;
    using allocator_type = Allocator;
    using node_traits = std::allocator_traits<Allocator>;

    using node_allocator = typename node_traits::template rebind_alloc<Node>;
    using node_alloc_traits = std::allocator_traits<node_allocator>;

    static constexpr bool RED = true;
    static constexpr bool BLACK = false;
    BaseNode _header;  // fake node. left the biggest element, right the smallest
    [[no_unique_address]] Compare _cmp;
    [[no_unique_address]] node_allocator _alloc;
    size_t _size = 0;

    template <bool IsConst>
    class base_iterator {
        using pointer_node_type = std::conditional_t<IsConst, const BaseNode *, BaseNode *>;

        pointer_node_type _node_ptr;
        friend class my_set;

        base_iterator(pointer_node_type node_ptr) : _node_ptr(node_ptr) {}

    public:
        using pointer_type = const Key *;
        using reference_type = const Key &;

        using value_type = Key;

        base_iterator(const base_iterator &) = default;
        base_iterator &operator=(const base_iterator &) = default;

        reference_type operator*() const {
            return static_cast<Node *>(_node_ptr)->key;
        }
        pointer_type operator->() const {
            return &(static_cast<Node *>(_node_ptr)->key);
        }

        base_iterator operator++() {
            incrementSuccessor(_node_ptr);
            return *this;
        }
        base_iterator operator--() {
            decrementSuccessor(_node_ptr);
            return *this;
        }

        bool operator==(const base_iterator &other) {
            return _node_ptr == other._node_ptr;
        }

        bool operator!=(const base_iterator &other) {
            return !(*this == other);
        }

        template <bool WasConst, typename = std::enable_if_t<IsConst && !WasConst>>
        base_iterator(const base_iterator<WasConst> &other) : _node_ptr(other._node_ptr) {}
    };

    static void incrementSuccessor(BaseNode *&node) {
        if (!node->right) {
            while (node->isOnRight() && node != node->parent->parent) node = node->parent;

            node = node->parent;

            return;
        }

        node = node->right;
        while (node->left) node = node->left;
    }

    static void decrementSuccessor(BaseNode *&node) {
        if (!node->left) {
            while (node->isOnLeft()) node = node->parent;
            node = node->parent;
            return;
        }

        node = node->left;
        while (node->right) node = node->right;
    }

    void leftRotate(BaseNode *leaf) {
        BaseNode *parent = leaf->parent;

        leaf->parent = parent->parent;
        parent->right = leaf->left;
        if (leaf->left) leaf->left->parent = parent;
        leaf->left = parent;
        parent->parent = leaf;

        if (leaf->parent == &_header)
            _header.parent = leaf;
        else if (leaf->parent->left == parent)
            leaf->parent->left = leaf;
        else
            leaf->parent->right = leaf;
    }

    void rightRotate(BaseNode *leaf) {
        BaseNode *parent = leaf->parent;

        leaf->parent = parent->parent;
        parent->left = leaf->right;
        if (leaf->right) leaf->right->parent = parent;
        leaf->right = parent;
        parent->parent = leaf;

        if (leaf->parent == &_header)
            _header.parent = leaf;
        else if (leaf->parent->left == parent)
            leaf->parent->left = leaf;
        else
            leaf->parent->right = leaf;
    }

    void fixInsert(BaseNode *leaf) {
        while (leaf != _header.parent && static_cast<Node *>(leaf->parent)->is_red) {
            BaseNode *gp = leaf->parent->parent;
            if (leaf->parent == gp->left) {                                 // if left child
                if (gp->right && static_cast<Node *>(gp->right)->is_red) {  // Check uncle color
                    static_cast<Node *>(leaf->parent)->is_red = BLACK;
                    static_cast<Node *>(gp->right)->is_red = BLACK;
                    static_cast<Node *>(gp)->is_red = RED;
                    leaf = gp;
                } else {
                    if (leaf == leaf->parent->right) {
                        leaf = leaf->parent;
                        leftRotate(leaf->right);
                    }
                    static_cast<Node *>(leaf->parent)->is_red = BLACK;
                    static_cast<Node *>(leaf->parent->parent)->is_red = RED;
                    rightRotate(leaf->parent);
                }
            } else {
                if (gp->left && static_cast<Node *>(gp->left)->is_red) {  // Check uncle color
                    static_cast<Node *>(leaf->parent)->is_red = BLACK;
                    static_cast<Node *>(gp->left)->is_red = BLACK;
                    static_cast<Node *>(gp)->is_red = RED;
                    leaf = gp;
                } else {
                    if (leaf == leaf->parent->left) {
                        leaf = leaf->parent;
                        rightRotate(leaf->left);
                    }
                    static_cast<Node *>(leaf->parent)->is_red = BLACK;
                    static_cast<Node *>(leaf->parent->parent)->is_red = RED;
                    leftRotate(leaf->parent);
                }
            }
            static_cast<Node *>(_header.parent)->is_red = BLACK;
        }
        ++_size;
    }

    void deleteFix(BaseNode *node_to_delete) {
        if (!node_to_delete->left && !node_to_delete->right) {  // no children
            if (node_to_delete == _header.parent) {
                _header.parent = nullptr;
                _header.right = &_header;  // begin == end
            } else {
                if (!(static_cast<Node *>(node_to_delete)->is_red)) {
                    fixDoubleBlack(node_to_delete);
                } else {
                    if (node_to_delete->sibling())
                        static_cast<Node *>(node_to_delete->sibling())->is_red = RED;
                }

                if (node_to_delete->isOnLeft()) {
                    node_to_delete->parent->left = nullptr;
                } else {
                    node_to_delete->parent->right = nullptr;
                }
            }

            // delete node_to_delete;
            node_alloc_traits::destroy(_alloc, static_cast<Node *>(node_to_delete));
            node_alloc_traits::deallocate(_alloc, static_cast<Node *>(node_to_delete), 1);
            --_size;
            return;
        } else {
            BaseNode *child = node_to_delete->left ? node_to_delete->left : node_to_delete->right;
            if (node_to_delete == _header.parent) {
                child->parent = &_header;
                _header.parent = child;
                static_cast<Node *>(node_to_delete)->is_red = BLACK;
                // delete node_to_delete;

                node_alloc_traits::destroy(_alloc, static_cast<Node *>(node_to_delete));
                node_alloc_traits::deallocate(_alloc, static_cast<Node *>(node_to_delete), 1);
            } else {
                if (node_to_delete->isOnLeft())
                    node_to_delete->parent->left = child;
                else
                    node_to_delete->parent->right = child;

                child->parent = node_to_delete->parent;

                if (!(static_cast<Node *>(child)->is_red) &&
                    !(static_cast<Node *>(node_to_delete)->is_red))
                    fixDoubleBlack(child);
                else
                    static_cast<Node *>(child)->is_red = BLACK;

                node_alloc_traits::destroy(_alloc, static_cast<Node *>(node_to_delete));
                node_alloc_traits::deallocate(_alloc, static_cast<Node *>(node_to_delete), 1);
            }
        }
        --_size;
    }
    void fixDoubleBlack(BaseNode *current) {
        while (current != _header.parent) {
            BaseNode *sibling = current->sibling();
            BaseNode *parent = current->parent;
            if (!sibling) {
                current = parent;
                continue;
            }

            if (static_cast<Node *>(sibling)->is_red) {
                static_cast<Node *>(parent)->is_red = RED;
                static_cast<Node *>(sibling)->is_red = BLACK;
                if (sibling->isOnLeft())
                    rightRotate(parent->left);
                else
                    leftRotate(parent->right);

            } else {
                if ((sibling->left && static_cast<Node *>(sibling->left)->is_red) ||
                    (sibling->right && static_cast<Node *>(sibling->right)->is_red)) {
                    if (sibling->left && static_cast<Node *>(sibling->left)->is_red) {
                        if (sibling->isOnLeft()) {
                            // left left
                            static_cast<Node *>(sibling->left)->is_red =
                                static_cast<Node *>(sibling)->is_red;
                            static_cast<Node *>(sibling)->is_red =
                                static_cast<Node *>(parent)->is_red;
                            rightRotate(parent->left);
                        } else {
                            // right left
                            static_cast<Node *>(sibling->left)->is_red =
                                static_cast<Node *>(parent)->is_red;
                            rightRotate(sibling->left);
                            leftRotate(parent->right);
                        }
                    } else {
                        if (sibling->isOnLeft()) {
                            // left right
                            static_cast<Node *>(sibling->right)->is_red =
                                static_cast<Node *>(parent)->is_red;
                            leftRotate(sibling->right);
                            rightRotate(parent->left);
                        } else {
                            // right right
                            static_cast<Node *>(sibling->right)->is_red =
                                static_cast<Node *>(sibling)->is_red;
                            static_cast<Node *>(sibling)->is_red =
                                static_cast<Node *>(parent)->is_red;
                            leftRotate(parent->right);
                        }
                    }
                    static_cast<Node *>(parent)->is_red = BLACK;
                    break;
                } else {
                    static_cast<Node *>(sibling)->is_red = RED;
                    if (!(static_cast<Node *>(parent)->is_red))
                        current = parent;
                    else {
                        static_cast<Node *>(parent)->is_red = BLACK;
                        break;
                    }
                }
            }
        }
    }

    void inorderCopyConstructor(BaseNode *node, BaseNode *parent) {
        if (node) {
            Node *new_node = node_alloc_traits::allocate(_alloc, 1);
            try {
                node_alloc_traits::construct(_alloc, new_node, static_cast<Node *>(node)->key,
                                             static_cast<Node *>(node)->is_red);
            } catch (...) {
                node_alloc_traits::deallocate(_alloc, new_node, 1);
            }
            new_node->parent = parent;
            if (node->isOnLeft())
                parent->left = new_node;
            else
                parent->right = new_node;

            if (_cmp(new_node->key, static_cast<Node *>(_header.right)->key))
                _header.right = new_node;
            if (_cmp(static_cast<Node *>(_header.left)->key, new_node->key))
                _header.left = new_node;

            inorderCopyConstructor(node->left, new_node);
            inorderCopyConstructor(node->right, new_node);
        }
    }

    void inorder(BaseNode *node) {
        if (node) {
            inorder(node->left);
            std::cout << static_cast<Node *>(node)->key << "(" << static_cast<Node *>(node)->is_red
                      << ") ";
            inorder(node->right);
        }
    }
    void deleteTrvl(BaseNode *node) {
        if (node) {
            deleteTrvl(node->left);
            deleteTrvl(node->right);

            node_alloc_traits::destroy(_alloc, static_cast<Node *>(node));
            node_alloc_traits::deallocate(_alloc, static_cast<Node *>(node), 1);
        }
    }

public:
    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using size_type = typename node_traits::size_type;
    using difference_type = typename node_traits::difference_type;
    using pointer = typename node_traits::pointer;
    using const_pointer = typename node_traits::const_pointer;
    using reference = value_type &;
    using const_reference = const value_type &;

    my_set(const my_set &other) {
        if (!other._header.parent) {
            _header.right = &_header;
            return;
        }

        Node *new_node = node_alloc_traits::allocate(_alloc, 1);
        try {
            node_alloc_traits::construct(_alloc, new_node,
                                         static_cast<Node *>(other._header.parent)->key, BLACK);
        } catch (...) {
            node_alloc_traits::deallocate(_alloc, new_node, 1);
        }

        _header.parent = new_node;
        _header.parent->parent = &_header;

        _header.left = _header.parent;
        _header.right = _header.parent;

        inorderCopyConstructor(other._header.parent->left, _header.parent);
        inorderCopyConstructor(other._header.parent->right, _header.parent);
        _size = other._size;
    }

    my_set() {
        _header.right = &_header;
    }

    iterator begin() {
        return {_header.right};
    }

    iterator end() {
        return {&_header};
    }

    const_iterator begin() const {
        return {_header.right};
    }

    const_iterator end() const {
        return {&_header};
    }

    void erase(const value_type &key) {
        BaseNode *current = _header.parent;
        while (current) {
            if (static_cast<Node *>(current)->key == key)
                break;
            else if (_cmp(static_cast<Node *>(current)->key, key))
                current = current->right;
            else
                current = current->left;
        }
        if (!current) return;

        if (!(current == _header.parent && !current->left && !current->right)) {  //
            // the smallest and the biggest node always have one child
            if (key == (static_cast<Node *>(_header.left))->key)
                decrementSuccessor(_header.left);
            else if (key == (static_cast<Node *>(_header.right))->key)
                incrementSuccessor(_header.right);
        }

        // handle node that have 2 children
        if (current->left && current->right) {
            BaseNode *successor = current;
            incrementSuccessor(successor);

            // relink
            BaseNode *tmp = successor->right;
            if (current->parent == &_header)
                current->parent->parent = successor;
            else if (current->isOnLeft())
                current->parent->left = successor;
            else
                current->parent->right = successor;

            successor->left = current->left;
            successor->right = current->right;
            successor->left->parent = successor;
            successor->right->parent = successor;

            if (successor->isOnLeft())
                successor->parent->left = current;
            else
                successor->parent->right = current;

            current->left = nullptr;
            current->right = tmp;
            if (tmp) tmp->parent = current;

            tmp = current->parent;
            current->parent = successor->parent;
            successor->parent = tmp;
        }

        deleteFix(current);
    }

    void insert(const value_type &key) {
        if (!_header.parent) {
            Node *new_node = node_alloc_traits::allocate(_alloc, 1);
            try {
                node_alloc_traits::construct(_alloc, new_node, key, BLACK);
            } catch (...) {
                node_alloc_traits::deallocate(_alloc, new_node, 1);
                throw;
            }
            _header.parent = new_node;
            _header.parent->parent = &_header;

            _header.left = _header.parent;
            _header.right = _header.parent;
            return;
        }
        BaseNode *current_key = _header.parent;
        while (current_key) {
            if (_cmp(static_cast<Node *>(current_key)->key, key)) {
                if (!current_key->right) {
                    Node *new_node = node_alloc_traits::allocate(_alloc, 1);
                    try {
                        node_alloc_traits::construct(_alloc, new_node, key, RED);
                    } catch (...) {
                        node_alloc_traits::deallocate(_alloc, new_node, 1);
                    }
                    current_key->right = new_node;
                    new_node->parent = current_key;
                    if (_cmp(new_node->key, static_cast<Node *>(_header.right)->key))
                        _header.right = new_node;
                    if (_cmp(static_cast<Node *>(_header.left)->key, new_node->key))
                        _header.left = new_node;
                    fixInsert(new_node);

                    break;
                }
                current_key = current_key->right;
            } else {
                if (!current_key->left) {
                    Node *new_node = node_alloc_traits::allocate(_alloc, 1);
                    try {
                        node_alloc_traits::construct(_alloc, new_node, key, RED);
                    } catch (...) {
                        node_alloc_traits::deallocate(_alloc, new_node, 1);
                    }
                    current_key->left = new_node;
                    new_node->parent = current_key;
                    if (_cmp(new_node->key, static_cast<Node *>(_header.right)->key))
                        _header.right = new_node;
                    if (_cmp(static_cast<Node *>(_header.left)->key, new_node->key))
                        _header.left = new_node;
                    fixInsert(new_node);

                    break;
                }
                current_key = current_key->left;
            }
        }
    }

    iterator upper_bound(const value_type &key) {
        BaseNode *current = _header.parent;
        while (current) {
            if (key == static_cast<Node *>(current)->key) break;
            if (_cmp(key, static_cast<Node *>(current)->key)) current = current->left;
            if (_cmp(static_cast<Node *>(current)->key, key)) current = current->right;
        }
        incrementSuccessor(current);
        return {current};
    }

    iterator lower_bound(const value_type &key) {
        BaseNode *current = _header.parent;
        while (current) {
            if (key == static_cast<Node *>(current)->key) return {current};
            if (_cmp(key, static_cast<Node *>(current)->key)) current = current->left;
            if (_cmp(static_cast<Node *>(current)->key, key)) current = current->right;
        }
        decrementSuccessor(current);
        return {current};
    }

    iterator find(const value_type &key) {
        BaseNode *current = _header.parent;
        while (current) {
            if (key == static_cast<Node *>(current)->key) return {current};
            if (_cmp(key, static_cast<Node *>(current)->key)) current = current->left;
            if (_cmp(static_cast<Node *>(current)->key, key)) current = current->right;
        }
        return end();
    }
    const_iterator find(const value_type &key) const {
        BaseNode *current = _header.parent;
        while (current) {
            if (key == static_cast<Node *>(current)->key) return {current};
            if (_cmp(key, static_cast<Node *>(current)->key)) current = current->left;
            if (_cmp(static_cast<Node *>(current)->key, key)) current = current->right;
        }
        return end();
    }
    size_t count(const value_type &key) {
        BaseNode *current = _header.parent;
        while (current) {
            if (key == static_cast<Node *>(current)->key) return 1;
            if (_cmp(key, static_cast<Node *>(current)->key)) current = current->left;
            if (_cmp(static_cast<Node *>(current)->key, key)) current = current->right;
        }
        return 0;
    }
    void clear() {
        deleteTrvl(_header.parent);
        _size = 0;
    }
    void inorderPrint() {
        inorder(_header.parent);
    }
    ~my_set() {
        clear();
    }
};
