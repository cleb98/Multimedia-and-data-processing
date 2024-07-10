#include <cstdint>
#include <utility>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <algorithm>

using rgb = std::array<uint8_t, 3>;
template <typename T>
class mat {
	int rows_, cols_;
	std::vector<T> data_;

public:
	mat(int r = 0, int c = 0) : cols_(c), rows_(r), data_(r* c) {};
	~mat() {};

	int cols() const {
		return cols_;
	}

	int rows() const {
		return rows_;
	}

	auto& data() {
		return data_;
	}

	int size() const {
		return rows_ * cols_;
	}

	void resize(int r, int c) {
		rows_ = r; 
		cols_ = c;
		data_.resize(r * c);
	}

	auto& operator() (int r, int c) {
		return data_[r*cols_ +c];
	}

	const auto& operator() (int r, int c) const{
		return data_[r * cols_ + c];
	}

	auto& operator[] (int i) {
		return data_[i];
	}

	auto begin() {
		return data_.begin();
	}
	auto rbegin() {
		return data_.rbegin();
	}
	auto end() {
		return data_.end();
	}
};

void error(const std::string& s) {
	std::cout << s;
	exit(EXIT_FAILURE);
}

template <typename T>
std::istream& rawread(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

struct bmp_header {
	uint16_t magic;
	uint32_t fsize;
	uint16_t res1;
	uint16_t res2;
	uint32_t offset;

};

struct bmp_info {
	uint32_t hsize;
	int32_t w;
	int32_t	h;
	uint16_t plane, bitppx;
	uint32_t method, img_size, hres, vres, num_colors, dummy;
};


bool load_bmp(std::istream& is, mat<rgb>& img) {
	using namespace std;

	bmp_header header;
	rawread(is, header.magic);
	if (header.magic != 0x4D42) {
		error("BMP identiefier is not BM");
	}
	rawread(is, header.fsize);
	rawread(is, header.res1);
	rawread(is, header.res2);
	rawread(is, header.offset); 

	//infoheader 
	uint32_t hsize;
	rawread(is, hsize);
	if (hsize != 40) {
		error("Wrong header size\n");
	}
	int32_t w, h;
	rawread(is, w);
	rawread(is, h);
	img.resize(h, w);

	uint16_t plane, bitppx;
	rawread(is, plane);
	if (plane != 1) {
		error("Wrong color planes number\n");
	}
	rawread(is, bitppx);
	if (bitppx != 24) {
		error("uncorrect number of bits per pixel");
	}
	uint32_t method, img_size, hres, vres, num_colors, dummy;
	rawread(is, method);
	if (method != 0) {
		error(" not BI_RGB compression used");
	}
	//discard 12 byte
	rawread(is, img_size);
	rawread(is, hres);
	rawread(is, vres);

	rawread(is, num_colors);
	if (num_colors == 0) {
		num_colors = pow(2, bitppx);
	}

	rawread(is, dummy);

	// 1)exctract reversed img
	int remaining = (img.cols() * 3) % 4;
	int padsize = 0;
	if (remaining > 0) {
		padsize = 4 - remaining;
	}

	for (int r = img.rows() - 1; r >= 0; r--) {
		for (int c = 0; c < img.cols(); c++) {
			img(r, c)[2] = is.get();  //B G R pad
			img(r, c)[1] = is.get();
			img(r, c)[0] = is.get();
		}
		if (!is)
			return false;
		for (int j = 0; j < padsize; j++) {
			is.get();
		}
	}

	return true;
}


bool savepam(std::ostream& os, const mat<rgb>& img) {
	using namespace std;
	/*
	P7
	WIDTH 227
	HEIGHT 149
	DEPTH 3
	MAXVAL 255
	TUPLTYPE RGB
	ENDHDR
	*/
	os << "P7\n";
	os << "WIDTH " << img.cols() << "\n";
	os << "HEIGHT " << img.rows() << "\n";
	os << "DEPTH 3\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE RGB\n";
	os << "ENDHDR\n";

	for (int r = 0; r < img.rows(); r++) {
		for (int c = 0; c < img.cols(); c++) {
			os.put(img(r, c)[0]);
			os.put(img(r, c)[1]);
			os.put(img(r, c)[2]);
		}
	}
	return true;
}


int main(int argc, char** argv) {

	if (argc != 3) {
		error("syntax error:\n bmp2pam <input file .BMP> <output file .PAM>\n");
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		error("impossible to read " + std::string(argv[1]));
	}

	mat<rgb> img;
	if (!load_bmp(is, img)) {
		error("impossible to load BMP file");
	}

	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		error("impossible to read " + std::string(argv[2]));
	}

	if (!savepam(os, img)) {
		error("impossible to save PAM file");
	}

	/*_________________*/
	return EXIT_SUCCESS;
}


