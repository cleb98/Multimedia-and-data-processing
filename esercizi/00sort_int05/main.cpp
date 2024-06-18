#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cassert>

int comp_int(const void *va, const void *vb)
{
    const int *ia = (const int *)va;
    const int *ib = (const int *)vb;

    if (*ia < *ib) {
        return -1;
    }
    else if (*ib < *ia) {
        return 1;
    }
    else {
        return 0;
    }
}

struct vector {
    size_t n_;
    size_t c_;
    int32_t* data_;

    vector() {
        n_ = 0;
        c_ = 8;
        data_ = (int32_t*) malloc(c_ * sizeof(int32_t));
    }
    ~vector() {
        free(data_);
    }
    void push_back(int32_t val) {
        n_++;
        if (n_ > c_) {
            c_ *= 2;
            data_ = (int32_t*) realloc(data_, c_ * sizeof(int32_t));
        }
        data_[n_ - 1] = val;
    }
    void shrink_to_fit() {
        data_ = (int32_t*) realloc(data_, n_ * sizeof(int32_t)); // shrink
        c_ = n_;
    }
    void sort() {
        qsort(data_, n_, sizeof(int32_t), comp_int);
    }
    size_t size() const {
        return n_;
    }
    int32_t& at(size_t pos) {
        assert(pos < n_);
        return data_[pos];
    }
    int32_t& operator[](size_t pos) {
        assert(pos < n_);
        return data_[pos];
    }
    //void set(size_t pos, int32_t val) {
    //    assert(pos < n_);
    //    data_[pos] = val;
    //}
};


int main(int argc, char *argv[])
{
    if (argc != 3) {
        return EXIT_FAILURE;
    }

    FILE *fin = fopen(argv[1], "r"); //<--'\r','\n'->'\n'
    if (fin == NULL) {
        return EXIT_FAILURE;
    }
    
    vector v;

    while (1) {
        int32_t val;
        int res = fscanf(fin, "%d", &val); // read
        if (res != 1) { // check
            break;
        }
        // use
        v.push_back(val);
    }
    v.shrink_to_fit();
    fclose(fin);

    v.sort();

    // Double the first element of the vector
    v[0] *= 2;
   
    FILE *fout = fopen(argv[2], "w");
    if (fout == NULL) {
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < v.size(); ++i) {
        fprintf(fout, "%d\n", v[i]);
    }
    fclose(fout);

    return EXIT_SUCCESS;
}