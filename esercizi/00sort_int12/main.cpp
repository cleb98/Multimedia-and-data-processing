#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cassert>

#include <utility>

template<typename T>
class vector {
    size_t n_;
    size_t c_;
    T* data_;

    void set_capacity(size_t newcapacity) {
        c_ = newcapacity;
        assert(c_ >= n_);
        // allocate new memory
        T* tmp = new T[c_];
        // copy old data into new memory
        for (size_t i = 0; i < n_; ++i) { // copy
            tmp[i] = data_[i];
        }
        // free old memory
        delete[] data_;
        // point to the new one
        data_ = tmp;
    }

public:
    friend void swap(vector& a, vector& b) {
        using std::swap;

        swap(a.n_, b.n_);
        swap(a.c_, b.c_);
        swap(a.data_, b.data_);
    }

    vector() // default constructor
        : n_(0), c_(8)
    { 
        printf("vector()\n");
        data_ = new T[c_];
    }
    vector(const vector& other) // copy constructor
        : n_(other.n_), c_(other.c_)
    {
        printf("vector(const vector& other)\n");
        data_ = new T[c_];
        for (size_t i = 0; i < n_; ++i) {
            data_[i] = other.data_[i];
        }
    }
    vector(vector&& other) // move constructor
        : n_(other.n_), c_(other.c_)
    {
        printf("vector(vector&& other)\n");
        data_ = other.data_;
        other.data_ = nullptr;
    }
    vector& operator=(vector rhs) { // assignment
        printf("vector& operator=(vector rhs)\n");
        swap(*this, rhs);
        return *this;
    }
    ~vector() { // destructor
        printf("~vector()\n");
        delete[] data_;
    }

    void push_back(const T& val) {
        if (n_ == c_) {
            set_capacity(c_ * 2);
        }
        ++n_;
        data_[n_ - 1] = val;
    }
    void shrink_to_fit() {
        set_capacity(n_);
    }
    void sort() {
        for (size_t i = 0; i < n_; ++i) {
            size_t minpos = i;
            for (size_t j = i + 1; j < n_; ++j) {
                if (data_[j] < data_[minpos]) {
                    minpos = j;
                }
            }
            T tmp = data_[i];
            data_[i] = data_[minpos];
            data_[minpos] = tmp;
        }
    }
    size_t size() const {
        return n_;
    }
    T& at(size_t pos) {
        assert(pos < n_);
        return data_[pos];
    }
    const T& at(size_t pos) const {
        assert(pos < n_);
        return data_[pos];
    }
    T& operator[](size_t pos) {
        return data_[pos];
    }
    const T& operator[](size_t pos) const {
        return data_[pos];
    }
};

vector<int> read_vector(const char *filename)
{
    vector<int> v;

    FILE *fin = fopen(filename, "r");
    if (fin == NULL) {
        return v;
    }

    while (1) {
        int val;
        int res = fscanf(fin, "%d", &val); // read
        if (res != 1) { // check
            break;
        }
        v.push_back(val); // use
    }
    v.shrink_to_fit();
    fclose(fin);
    return v;
}

bool write_vector(const char *filename, const vector<int>& v) {
    FILE *fout = fopen(filename, "w");
    if (fout == NULL) {
        return false;
    }
    for (size_t i = 0; i < v.size(); ++i) {
        fprintf(fout, "%d\n", v[i]);
    }
    fclose(fout);
    return true;
}

int main(int argc, char *argv[])
{
    {
        if (argc != 3) {
            return EXIT_FAILURE;
        }

        vector<int> v;
        
        v = read_vector(argv[1]);

        if (v.size() == 0) {
            return EXIT_FAILURE;
        }

        v.sort();

        // Double the first element of the vector
        v[0] *= 2;

        vector<int> x;
        x = v;

        write_vector(argv[2], v);
    }
    _CrtDumpMemoryLeaks();
    return EXIT_SUCCESS;
}
