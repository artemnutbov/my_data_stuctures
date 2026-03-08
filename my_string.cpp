#include <compare>
#include <cstring>
#include <iostream>

class myString {
    char* str = nullptr;
    size_t sz = 0;
    size_t cap = 0;
    void constructLps(const myString&, size_t*) const;

public:
    static const size_t npos = -1;

    myString() = default;
    myString(const char*);
    myString(size_t, char);
    myString(const myString&);
    myString& operator=(myString);
    myString& operator=(const char*);
    myString& operator=(char);
    void swap(myString&);

    // spaceship operator
    std::weak_ordering operator<=>(const myString& other) const {
        int cmp = std::strcmp(str, other.str);
        if (cmp < 0) return std::weak_ordering::less;
        if (cmp > 0) return std::weak_ordering::greater;
        return std::weak_ordering::equivalent;
    }
    std::weak_ordering operator<=>(const char* s) const {
        int cmp = std::strcmp(str, s);
        if (cmp < 0) return std::weak_ordering::less;
        if (cmp > 0) return std::weak_ordering::greater;
        return std::weak_ordering::equivalent;
    }
    bool operator==(const myString& other) const {
        if (sz != other.sz) return false;
        if (this == &other) return true;
        if (std::strcmp(str, other.str) == 0) return true;
        return false;
    }
    bool operator==(const char* s) const {
        if (sz != strlen(s)) return false;
        if (str == s) return true;
        if (std::strcmp(str, s) == 0) return true;
        return false;
    }

    size_t size() const {
        return sz;
    }
    size_t capacity() const {
        return cap;
    }

    char& operator[](size_t x) {
        return str[x];
    }
    const char& operator[](size_t x) const {
        return str[x];
    }
    char& at(size_t);
    const char& at(size_t) const;
    char& front();
    const char& front() const;
    char& back();
    const char& back() const;

    void resize(size_t);
    void resize(size_t, char);  // need to write all function

    char* begin() {
        return str;
    }
    char* end() {
        return str + sz;
    }
    const char* begin() const {
        return str;
    }
    const char* end() const {
        return str + sz;
    }

    void reserve(size_t);

    void clear();
    bool empty() const;
    void shrink_to_fit();

    myString& operator+=(const myString&);
    myString& operator+=(const char*);
    myString& operator+=(char);

    myString& append(const myString&);
    myString& append(const myString&, size_t, size_t);
    myString& append(const char*);
    myString& append(const char*, size_t);
    myString& append(size_t, char);

    void push_back(char);
    void pop_back();
    myString& erase(size_t, size_t);

    myString& assign(const myString&);
    myString& assign(const myString&, size_t, size_t);
    myString& assign(const char*);
    myString& assign(const char*, size_t);
    myString& assign(size_t, char);

    myString& insert(size_t, const myString&);
    myString& insert(size_t, const myString&, size_t, size_t);
    myString& insert(size_t, const char*);
    myString& insert(size_t, const char*, size_t);
    myString& insert(size_t, size_t, char);
    myString& insert(size_t, char);

    myString& replace(size_t, size_t, const myString&);
    myString& replace(size_t, size_t, const myString&, size_t, size_t);
    myString& replace(size_t, size_t, const char*);
    myString& replace(size_t, size_t, const char*, size_t);
    myString& replace(size_t, size_t, size_t, char);

    const char* c_str() const {
        return str;
    }
    const char* data() const {
        return str;
    }

    size_t copy(char*, size_t, size_t) const;

    size_t find(const myString&, size_t) const;
    size_t find(const char*, size_t) const;
    size_t find(const char*, size_t, size_t) const;
    size_t find(char, size_t) const;

    size_t rfind(const myString&, size_t) const;
    size_t rfind(const char*, size_t) const;
    size_t rfind(const char*, size_t, size_t) const;
    size_t rfind(char, size_t) const;

    size_t find_first_of(const myString&, size_t) const;
    size_t find_last_of(const myString&, size_t) const;
    size_t find_first_not_of(const myString&, size_t) const;
    size_t find_last_not_of(const myString&, size_t) const;

    myString substr(size_t, size_t) const;
    ~myString() {
        if (str) delete[] str;
    }
};

myString::myString(const char* s) {
    if (s) {
        sz = strlen(s);
        cap = sz;
        str = new char[sz + 1];
        strcpy(str, s);
        str[sz] = '\0';
    } else {
        str = new char[1];
        sz = 0;
        cap = sz;
        str[0] = '\0';
    }
}
myString::myString(const myString& other)
    : str(new char[other.sz + 1]), sz(other.sz), cap(other.cap) {
    memcpy(str, other.str, sz + 1);
}
myString::myString(size_t n, char c) : str(new char[n + 1]), sz(n), cap(n) {
    memset(str, c, sz + 1);
    str[sz] = '\0';
}
myString& myString::operator=(myString other) {
    swap(other);
    return *this;
}
myString& myString::operator=(const char* s) {
    if (!s) return *this;
    if (str) delete[] str;
    str = new char[strlen(s) + 1];
    sz = strlen(s);
    cap = sz;
    memcpy(str, s, sz + 1);
    return *this;
}
myString& myString::operator=(char c) {
    if (str) delete[] str;
    str = new char[2];
    sz = 1;
    cap = 1;
    str[0] = c;
    str[1] = '\0';
    return *this;
}

void myString::swap(myString& other) {
    std::swap(str, other.str);
    std::swap(sz, other.sz);
    std::swap(cap, other.cap);
}
void myString::resize(size_t n) {  // warning not sure
    // if(sz == cap) return;
    if (n > cap) {
        size_t i = sz;
        while (i < n) {
            i *= 2;
        }
        reserve(i);
        memset(str + sz, '\0', n - sz);
        cap = i;
        sz = n;
    } else if (n > sz && n < cap) {
        sz = n;
    } else {
        sz = n;
        str[sz] = '\0';
    }
}
void myString::resize(size_t n, char c) {
    if (n >= cap) {
        size_t i = sz;
        while (i < n) {
            i *= 2;
        }
        reserve(i);
        cap = i;
        memset(str + sz, c, n - sz);
        sz = n;
        str[sz] = '\0';
    } else if (n > sz) {
        memset(str + sz, c, n - sz);
        sz = n;
        str[sz] = '\0';
    } else {
        sz = n;
        str[sz] = '\0';
    }
}
void myString::reserve(size_t n) {
    if (n <= cap) return;
    if (!str) {
        str = new char[n];
        cap = n;
        return;
    }
    myString tmp = *this;
    delete[] str;
    str = new char[n];
    memcpy(str, tmp.str, sz + 1);
    str[sz] = '\0';
    cap = n;
}
void myString::clear() {
    sz = 0;
    str[sz] = '\0';
}
bool myString::empty() const {
    if (0 == sz) return true;
    return false;
}
void myString::shrink_to_fit() {
    if (sz == cap) return;
    myString tmp = *this;
    delete[] str;
    str = new char[sz + 1];
    memcpy(str, tmp.str, sz + 1);
    str[sz] = '\0';
    cap = sz;
}

char& myString::at(size_t x) {
    if (x < sz) return str[x];
    throw std::out_of_range("out of range");
}
const char& myString::at(size_t x) const {
    if (x < sz) return str[x];
    throw std::out_of_range("out of range");
}
char& myString::front() {
    return str[0];
}
const char& myString::front() const {
    return str[0];
}
char& myString::back() {
    if (sz == strlen(str)) return str[strlen(str) - 1];
    return str[strlen(str)];
}
const char& myString::back() const {
    if (sz == strlen(str)) return str[strlen(str) - 1];
    return str[strlen(str)];
}

myString operator+(const myString& lhs, const myString& rhs) {
    myString tmp(lhs.size() + rhs.size(), '\0');
    memcpy(tmp.begin(), lhs.begin(), lhs.size());
    memcpy(tmp.begin() + lhs.size(), rhs.begin(), rhs.size() + 1);
    return tmp;
}
myString operator+(const myString& lhs, const char* c) {
    myString tmp(lhs.size() + strlen(c), '\0');
    memcpy(tmp.begin(), lhs.begin(), lhs.size());
    memcpy(tmp.begin() + lhs.size(), c, strlen(c) + 1);
    return tmp;
}
myString operator+(const myString& lhs, char c) {
    myString tmp(lhs.size() + 1, '\0');
    memcpy(tmp.begin(), lhs.begin(), lhs.size());
    *(tmp.end() - 1) = c;
    return tmp;
}
myString operator+(char c, const myString& lhs) {
    myString tmp(lhs.size() + 1, c);
    memcpy(tmp.begin() + 1, lhs.begin(), lhs.size() + 1);
    return tmp;
}

myString& myString::operator+=(const myString& other) {
    *this = *this + other;
    return *this;
}
myString& myString::operator+=(const char* s) {
    *this = *this + s;
    return *this;
}
myString& myString::operator+=(char c) {
    *this = *this + c;
    return *this;
}

myString& myString::append(const myString& other) {
    *this = *this + other;
    return *this;
}
myString& myString::append(const myString& other, size_t subpos, size_t sublen = npos) {
    if (0 == sublen) return *this;
    if (subpos >= other.sz) throw std::out_of_range("out of range");
    if (sublen > other.sz - subpos) {
        myString tmp(sz + other.sz - subpos, '\0');
        swap(tmp);
        memcpy(str, tmp.str, tmp.sz);
        memcpy(str + tmp.sz, other.str + subpos, other.sz - subpos);
        return *this;
    }
    myString tmp(sz + sublen, '\0');
    swap(tmp);
    memcpy(str, tmp.str, tmp.sz);
    memcpy(str + tmp.sz, other.str + subpos, sublen);
    return *this;
}
myString& myString::append(const char* s) {
    *this = *this + s;
    return *this;
}
myString& myString::append(const char* s, size_t n) {
    if (0 == n) return *this;
    if (n - 1 > strlen(s)) throw std::out_of_range("out of range");
    myString tmp(sz + n - 1, '\0');
    swap(tmp);
    memcpy(str, tmp.str, tmp.sz);
    memcpy(str + tmp.sz, s, n - 1);
    return *this;
}
myString& myString::append(size_t n, char c) {
    if (0 == n) return *this;
    myString tmp(sz + n, c);
    swap(tmp);
    memcpy(str, tmp.str, tmp.sz);
    return *this;
}

void myString::push_back(char c) {
    *this = *this + c;
}
void myString::pop_back() {
    --sz;
    str[sz] = '\0';
}

myString& myString::assign(const myString& other) {
    *this = other;
    return *this;
}
myString& myString::assign(const myString& other, size_t subpos, size_t sublen = npos) {
    if (0 == sublen) return *this;
    if (subpos >= other.sz) throw std::out_of_range("out of range");
    if (sublen > other.sz - subpos) {
        myString tmp(other.sz - subpos, '\0');
        swap(tmp);
        memcpy(str, other.str + subpos, other.sz - subpos);
        return *this;
    }
    myString tmp(sublen, '\0');
    swap(tmp);
    memcpy(str, other.str + subpos, sublen);
    return *this;
}
myString& myString::assign(const char* s) {
    *this = s;
    return *this;
}
myString& myString::assign(const char* s, size_t n) {
    if (0 == n) return *this;
    if (n - 1 > strlen(s)) throw std::out_of_range("out of range");
    myString tmp(n, '\0');
    swap(tmp);
    memcpy(str, s, n);
    return *this;
}
myString& myString::assign(size_t n, char c) {
    if (0 == n) return *this;
    myString tmp(n, c);
    swap(tmp);
    return *this;
}

myString& myString::insert(size_t pos, const myString& other) {
    if (pos > sz) throw std::out_of_range("out of range");
    myString tmp(sz + other.sz, '\0');
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memcpy(str + pos, other.str, other.sz);
    memcpy(str + pos + other.sz, tmp.str + pos, tmp.sz - pos + 1);
    return *this;
}
myString& myString::insert(size_t pos, const myString& other, size_t subpos, size_t sublen) {
    if (0 == sublen) return *this;
    if (subpos >= other.sz || pos > sz) throw std::out_of_range("out of range");
    if (sublen > other.sz - subpos) {
        myString tmp(sz + other.sz - subpos, '\0');
        swap(tmp);
        memcpy(str, tmp.str, pos);
        memcpy(str + pos, other.str + subpos, other.sz - subpos);
        memcpy(str + pos + other.sz - subpos, tmp.str + pos, tmp.sz - pos + 1);
        return *this;
    }
    myString tmp(sz + sublen, '\0');
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memcpy(str + pos, other.str + subpos, sublen);
    memcpy(str + pos + sublen, tmp.str + pos, tmp.sz - pos + 1);
    return *this;
}
myString& myString::insert(size_t pos, const char* s) {
    size_t sz_s = strlen(s);
    if (pos > sz_s) throw std::out_of_range("out of range");
    myString tmp(sz + sz_s, '\0');
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memcpy(str + pos, s, sz_s);
    memcpy(str + pos + sz_s, tmp.str + pos, tmp.sz - pos + 1);
    return *this;
}
myString& myString::insert(size_t pos, const char* s, size_t n) {
    size_t sz_s = strlen(s);
    if (pos > sz || n > sz_s) throw std::out_of_range("out of range");
    myString tmp(sz + n, '\0');
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memcpy(str + pos, s, n);
    memcpy(str + n + pos, tmp.str + pos, tmp.sz - pos + 1);
    return *this;
}
myString& myString::insert(size_t pos, size_t n, char c) {
    if (pos > sz) throw std::out_of_range("out of range");
    myString tmp(sz + n, c);
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memcpy(str + n + pos, tmp.str + pos, tmp.sz - pos + 1);
    return *this;
}
myString& myString::insert(size_t pos, char c) {
    if (pos > sz) throw std::out_of_range("out of range");
    myString tmp(sz + 1, c);
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memcpy(str + 1 + pos, tmp.str + pos, tmp.sz + 1 - pos);
    return *this;
}

myString& myString::erase(size_t pos = 0, size_t n = npos) {
    if (pos > sz) throw std::out_of_range("out of range");
    if (n > sz - pos) {
        str[pos] = '\0';
        sz = pos;
        return *this;
    }
    memcpy(str + pos, str + pos + n, sz - pos - n);
    str[sz - n] = '\0';
    sz -= n;
    return *this;
}

myString& myString::replace(size_t pos, size_t n, const myString& other) {
    if (pos > sz) throw std::out_of_range("out of range");
    if (n > sz - pos) {
        myString tmp(sz + other.sz, '\0');
        swap(tmp);
        memcpy(str, tmp.str, pos);
        memcpy(str + pos, other.str, other.sz);
        str[pos + other.sz] = '\0';
        sz = pos + other.sz;
        return *this;
    }
    myString tmp(sz - n + other.sz, '\0');
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memcpy(str + pos, other.str, other.sz);
    memcpy(str + pos + other.sz, tmp.str + pos + n, tmp.sz - pos - n);
    return *this;
}
myString& myString::replace(size_t pos, size_t n, const myString& other, size_t subpos,
                            size_t sublen) {
    if (subpos >= other.sz || pos > sz) throw std::out_of_range("out of range");
    if (n > sz - pos) {
        if (sublen > other.sz - subpos) {
            myString tmp(sz + other.sz - subpos, '\0');
            swap(tmp);
            memcpy(str, tmp.str, pos);
            memcpy(str + pos, other.str + subpos, other.sz - subpos);
            str[pos + other.sz - subpos] = '\0';
            sz = pos + other.sz - subpos;
            return *this;
        }
        myString tmp(sz + sublen, '\0');
        swap(tmp);
        memcpy(str, tmp.str, pos);
        memcpy(str + pos, other.str + subpos, sublen);
        str[pos + sublen] = '\0';
        sz = pos + sublen;
        return *this;
    }
    if (sublen > other.sz - subpos) {
        myString tmp(sz - n + other.sz - subpos, '\0');
        swap(tmp);
        memcpy(str, tmp.str, pos);
        memcpy(str + pos, other.str + subpos, other.sz - subpos);
        memcpy(str + pos + other.sz - subpos, tmp.str + pos + n, tmp.sz - pos - n);
        return *this;
    }
    myString tmp(sz - n + sublen, '\0');
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memcpy(str + pos, other.str + subpos, sublen);
    memcpy(str + pos + sublen, tmp.str + pos + n, tmp.sz - pos - n);
    return *this;
}
myString& myString::replace(size_t pos, size_t n, const char* s) {
    size_t sz_s = strlen(s);
    if (pos > sz) throw std::out_of_range("out of range");
    if (n > sz - pos) {
        myString tmp(sz + sz_s, '\0');
        swap(tmp);
        memcpy(str, tmp.str, pos);
        memcpy(str + pos, s, sz_s);
        str[pos + sz_s] = '\0';
        sz = pos + sz_s;
        return *this;
    }
    myString tmp(sz - n + sz_s, '\0');
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memcpy(str + pos, s, sz_s);
    memcpy(str + pos + sz_s, tmp.str + pos + n, tmp.sz - pos - n);
    return *this;
}
myString& myString::replace(size_t pos, size_t n, const char* s, size_t len) {
    size_t sz_s = strlen(s);
    if (pos > sz || len > sz_s) throw std::out_of_range("out of range");
    if (0 == len) {
        return this->erase(pos, n);
    }
    if (n > sz - pos) {
        myString tmp(sz + len, '\0');
        swap(tmp);
        memcpy(str, tmp.str, pos);
        memcpy(str + pos, s, len);
        str[pos + len] = '\0';
        sz = pos + len;
        return *this;
    }
    myString tmp(sz - n + len, '\0');
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memcpy(str + pos, s, len);
    memcpy(str + pos + len, tmp.str + pos + n, tmp.sz - pos - n);
    return *this;
}
myString& myString::replace(size_t pos, size_t n, size_t len, char c) {
    if (pos > sz) throw std::out_of_range("out of range");
    if (n > sz - pos) {
        myString tmp(sz + len, '\0');
        swap(tmp);
        memcpy(str, tmp.str, pos);
        memset(str + pos, c, len);
        str[pos + len] = '\0';
        sz = pos + len;
        return *this;
    }
    myString tmp(sz - n + len, '\0');
    swap(tmp);
    memcpy(str, tmp.str, pos);
    memset(str + pos, c, len);
    memcpy(str + pos + len, tmp.str + pos + n, tmp.sz - pos - n);
    return *this;
}

size_t myString::copy(char* s, size_t len, size_t pos = 0) const {
    if (pos > sz || len > sz - pos) throw std::out_of_range("out of range");
    memcpy(s, str + pos, len);
    return len;
}

void myString::constructLps(const myString& other, size_t* lps) const {
    size_t len = 0;
    lps[0] = 0;
    size_t i = 1;
    while (i < other.sz) {
        if (other[i] == other[len]) {
            ++len;
            lps[i] = len;
            ++i;
        } else {
            if (len != 0)
                len = lps[len - 1];
            else {
                lps[i] = 0;
                ++i;
            }
        }
    }
}
size_t myString::find(const myString& other, size_t pos = 0) const {
    if (pos > sz) throw std::out_of_range("out of range");
    if (other.sz > sz) return -1;
    size_t* lps = new size_t[other.sz];

    constructLps(other, lps);
    size_t i = pos, j = 0;
    while (i < sz) {
        if (str[i] == other.str[j]) {
            ++i;
            ++j;
            if (j == other.sz) {
                delete[] lps;
                return i - j;
            }
        } else {
            if (0 != j)
                j = lps[j - 1];
            else
                ++i;
        }
    }
    delete[] lps;
    return -1;
}
size_t myString::find(const char* s, size_t pos = 0) const {
    size_t len_s = strlen(s);
    if (pos > sz) throw std::out_of_range("out of range");
    if (len_s > sz) return -1;

    size_t* lps = new size_t[len_s];

    size_t len = 0;
    lps[len] = 0;
    size_t lps_i = 1;
    while (lps_i < len_s) {
        if (s[lps_i] == s[len]) {
            ++len;
            lps[lps_i] = len;
            ++lps_i;
        } else {
            if (0 == len)
                lps[lps_i++] = 0;
            else
                len = lps[len - 1];
        }
    }
    size_t i = pos, j = 0;
    while (i < sz) {
        if (str[i] == s[j]) {
            ++i;
            ++j;
            if (j == len_s) {
                delete[] lps;
                return i - j;
            }
        } else {
            if (0 != j)
                j = lps[j - 1];
            else
                ++i;
        }
    }
    delete[] lps;
    return -1;
}
size_t myString::find(const char* s, size_t pos, size_t n) const {
    size_t len_s = strlen(s);
    if (pos > sz) throw std::out_of_range("out of range");
    if (len_s > sz || n > len_s) return -1;

    size_t* lps = new size_t[n];

    size_t len = 0;
    lps[len] = 0;
    size_t lps_i = 1;
    while (lps_i < n) {
        if (s[lps_i] == s[len]) {
            ++len;
            lps[lps_i] = len;
            ++lps_i;
        } else {
            if (0 == len)
                lps[lps_i++] = 0;
            else
                len = lps[len - 1];
        }
    }
    size_t i = pos, j = 0;
    while (i < sz) {
        if (str[i] == s[j]) {
            ++i;
            ++j;
            if (j == n) {
                delete[] lps;
                return i - j;
            }
        } else {
            if (0 != j)
                j = lps[j - 1];
            else
                ++i;
        }
    }
    delete[] lps;
    return -1;
}
size_t myString::find(char c, size_t pos = 0) const {
    if (pos > sz) throw std::out_of_range("out of range");
    size_t i = pos;
    while (i < sz) {
        if (c == str[i]) return i;
        ++i;
    }
    return -1;
}

size_t myString::rfind(const myString& other, size_t pos = 0) const {
    if (pos > sz) throw std::out_of_range("out of range");
    if (other.sz > sz) return -1;
    size_t* lps = new size_t[other.sz];

    constructLps(other, lps);
    size_t i = 0, j = 0;
    size_t last = npos;
    while (i < sz) {
        if (str[i] == other.str[j]) {
            ++i;
            ++j;
            if (j == other.sz) {
                last = i - j;
            }
            if (i > pos) {
                if (npos == last) return -1;
                return last;
            }
        } else {
            if (0 != j)
                j = lps[j - 1];
            else
                ++i;
            if (i > pos) {
                if (npos == last) return -1;
                return last;
            }
        }
    }
    delete[] lps;
    if (npos == last) return -1;
    return last;
}
size_t myString::rfind(const char* s, size_t pos = 0) const {
    size_t len_s = strlen(s);
    if (pos > sz) throw std::out_of_range("out of range");
    if (len_s > sz) return -1;

    size_t* lps = new size_t[len_s];
    size_t last = npos;
    size_t len = 0;
    lps[len] = 0;
    size_t lps_i = 1;
    while (lps_i < len_s) {
        if (s[lps_i] == s[len]) {
            ++len;
            lps[lps_i] = len;
            ++lps_i;
        } else {
            if (0 == len)
                lps[lps_i++] = 0;
            else
                len = lps[len - 1];
        }
    }
    size_t i = 0, j = 0;
    while (i < sz) {
        if (str[i] == s[j]) {
            ++i;
            ++j;
            if (j == len_s) {
                last = i - j;
            }
            if (i > pos) {
                if (npos == last) return -1;
                return last;
            }
        } else {
            if (0 != j)
                j = lps[j - 1];
            else
                ++i;
            if (i > pos) {
                if (npos == last) return -1;
                return last;
            }
        }
    }
    delete[] lps;

    if (npos == last) return -1;
    return last;
}
size_t myString::rfind(const char* s, size_t pos, size_t n) const {
    size_t len_s = strlen(s);
    if (pos > sz) throw std::out_of_range("out of range");
    if (len_s > sz || n > len_s) return -1;

    size_t* lps = new size_t[n];

    size_t len = 0;
    lps[len] = 0;
    size_t last = npos;
    size_t lps_i = 1;
    while (lps_i < n) {
        if (s[lps_i] == s[len]) {
            ++len;
            lps[lps_i] = len;
            ++lps_i;
        } else {
            if (0 == len)
                lps[lps_i++] = 0;
            else
                len = lps[len - 1];
        }
    }
    size_t i = 0, j = 0;
    while (i < sz) {
        if (str[i] == s[j]) {
            ++i;
            ++j;
            if (j == n) {
                last = i - j;
            }
            if (i > pos) {
                if (npos == last) return -1;
                return last;
            }
        } else {
            if (0 != j)
                j = lps[j - 1];
            else
                ++i;
            if (i > pos) {
                if (npos == last) return -1;
                return last;
            }
        }
    }
    delete[] lps;
    if (npos == last) return -1;
    return last;
}
size_t myString::rfind(char c, size_t pos = 0) const {
    if (pos > sz) throw std::out_of_range("out of range");
    size_t i = 0;
    size_t last = npos;
    while (i < sz) {
        if (c == str[i]) last = i;
        ++i;
        if (i > pos) {
            if (npos == last) return -1;
            return last;
        }
    }
    if (npos == last) return -1;
    return last;
}
size_t myString::find_first_of(const myString& other, size_t pos = 0) const {
    if (sz <= pos) throw std::out_of_range("out of range");
    for (size_t i = pos; i < sz; ++i) {
        for (size_t j = 0; j < other.sz; ++j)
            if (str[i] == other.str[j]) return i;
    }
    return -1;
}
size_t myString::find_last_of(const myString& other, size_t pos = npos) const {
    if (sz <= pos) pos = sz - 1;
    for (size_t i = pos; i != 0; --i) {
        for (size_t j = 0; j < other.sz; ++j)
            if (str[i] == other.str[j]) return i;
    }
    return -1;
}
size_t myString::find_first_not_of(const myString& other, size_t pos = 0) const {
    if (sz <= pos) throw std::out_of_range("out of range");
    for (size_t i = pos; i < sz; ++i) {
        bool tmp = false;
        for (size_t j = 0; j < other.sz; ++j) {
            if (str[i] == other.str[j]) {
                tmp = true;
                break;
            }
        }
        if (!tmp) return i;
    }
    return -1;
}
size_t myString::find_last_not_of(const myString& other, size_t pos = npos) const {
    if (sz <= pos) pos = sz - 1;
    for (size_t i = pos; i != 0; --i) {
        bool tmp = false;
        for (size_t j = 0; j < other.sz; ++j) {
            if (str[i] == other.str[j]) {
                tmp = true;
                break;
            }
        }
        if (!tmp) return i;
    }
    return -1;
}

myString myString::substr(size_t pos = 0, size_t len = npos) const {
    if (sz <= len - pos) {
        myString tmp(sz - pos, '\0');
        memcpy(tmp.str, str + pos, sz - pos);
        return tmp;
    }
    myString tmp(sz - len, '\0');
    memcpy(tmp.str, str + pos, len);
    return tmp;
}

std::ostream& operator<<(std::ostream& os, const myString& s) {
    for (auto i = s.begin(); *i != '\0'; ++i) {
        os << *i;
    }
    return os;
}
std::istream& operator>>(std::istream& is, myString& s) {
    char buffer[1024];
    is >> buffer;
    s = buffer;
    return is;
}

int main() {}