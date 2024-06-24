#include <vector>
#include <iostream>
#include <fstream>
#include <utility>
#include <array>
#include <algorithm>
#include <cstddef> 
#include <iterator>

template <typename T>
class  mat {

	int c_, r_;
	std::vector<T> data_;

public:

	mat(int rows = 0, int cols = 0) : c_(cols), r_(rows), data_(cols* rows) {}
	//getter
	int cols() const {
		return c_;
	}
	int rows() const {
		return r_;
	}

	const auto& data() const {
		return data_;
	}
	auto& data() {
		return data_;
	}
	//size 
	int size() const {
		return r_ * c_;
	}
	//resize
	void resize(int new_r, int new_c) {
		c_ = new_c,
		r_ = new_r;
		data_.resize(new_r * new_c);
	}
	//rawdata 
	char* rawdata() {
		return reinterpret_cast<char*> (data_.data());
	}


};




int main(int argc, char* argv[]) {


	
}