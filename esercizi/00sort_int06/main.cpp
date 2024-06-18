#define _CRT_SECURE_NO_WARNINGS
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

    vector() { //default constructor
        n_ = 0;
        c_ = 8;
        data_ = NULL;
    }
    // copy constructor,
    vector(const vector& other) { // same thing of assignment(=) but with another syntax 
        n_ = other.n_;
        c_ = other.c_;
        //data_ = (int32_t*)malloc(c_ * sizeof(int32_t));
        data_ = new int32_t[c_];
        for (size_t i = 0; i < n_; ++i) {
            data_[i] = other.data_[i];
        }
    }
    // assignment operator
    vector& operator=(const vector& rhs) { // rhs = right hand side
        if (rhs.c_ > c_) { 
			c_ = rhs.c_;
			//data_ = (int32_t*)realloc(data_, c_ * sizeof(int32_t));
			delete[] data_;
			data_ = new int32_t[c_];
        n_ = rhs.n_;

        for (size_t i = 0; i < n_; ++i) {
			data_[i] = rhs.data_[i];
		}
		}
    		return *this;
    	}

    ~vector() {
        puts("~vector()");
        //free(data_);
        delete[] data_;
    }
    void push_back(int32_t val) {
        n_++; // increment the number of elements hold by the buffer
        if (n_ > c_) {
            c_ = c_*2 +1;// double the capacity when the buffer is full
            //data_ = (int32_t*) realloc(data_, c_ * sizeof(int32_t));
            //implement the realloc job
            int32_t* tmp = new int32_t[c_]; // allocate new memory (a new buffer)
            for (size_t i = 0; i < n_ - 1; ++i) {
                tmp[i] = data_[i];// assigning the old smaller buffer to the new bigger buffer
            }
            delete [] data_; // free the old buffer
            data_ = tmp; //let the data_ buffer point to the new buffer
        }
        data_[n_ - 1] = val;
    }
    void shrink_to_fit() {
        c_ = n_;
        //data_ = (int32_t*) realloc(data_, n_ * sizeof(int32_t)); 
        //implement the realloc job
        int32_t* tmp = new int32_t[c_]; // allocate new memory (a new buffer)
        for (size_t i = 0; i < n_; ++i) {
			tmp[i] = data_[i];// assigning the old smaller buffer to the new bigger buffer
		}
        delete [] data_; // free the old buffer
        data_ = tmp; //let the data_ buffer point to the new buffer
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

    vector x = v;
   
    //FILE *fout = fopen(argv[2], "w");
    //if (fout == NULL) {
    //    return EXIT_FAILURE;
    //}
    for (size_t i = 0; i < x.size(); ++i) {
        fprintf(stdout, "%d\n", x[i]);
    }
    //fclose(fout);

    return EXIT_SUCCESS;
}