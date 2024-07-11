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

class bitreader {
	uint8_t buffer_ = 0;
	size_t n_ = 0;
	std::istream& is_;

	uint8_t readbit() {
		if (n_ == 0) {
			rawread(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::istream& is) :
		is_(is) {};

	~bitreader() {};

	uint64_t operator() (size_t bits) {
		uint64_t u = 0;
		while (bits-- > 0) {
			u = (u << 1) | readbit();
		}
		return u;
	}
};


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


bool load_bmp24(std::istream& is, mat<rgb>& img) {
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

bool load_bmp8(std::istream& is, mat<rgb>& img) {
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
	if (bitppx != 8) {
		error("uncorrect number of bits per pixel");
	}
	uint32_t method, img_size, hres, vres, num_colors, dummy;
	rawread(is, method);
	//if (method != 0) {
	//	error(" not BI_RGB compression used");
	//}
	//discard 12 byte
	rawread(is, img_size);
	rawread(is, hres);
	rawread(is, vres);

	rawread(is, num_colors);
	if (num_colors == 0) {
		num_colors = pow(2, bitppx);
	}

	rawread(is, dummy);
	//read table 
	using quad = array<uint8_t, 4>;
	std::vector<quad> ctable;

	quad pxl;
	for (int i = 0; i < num_colors; i++) {
		pxl[0] = is.get();
		pxl[1] = is.get();
		pxl[2] = is.get();
		pxl[3] = is.get();
		ctable.push_back(pxl);
	}

	// 1)exctract reversed img
	int remaining = (img.cols()) % 4;
	int padsize = 0;
	if (remaining > 0) {
		padsize = 4 - remaining;
	}

	uint8_t index;
	for (int r = img.rows() - 1; r >= 0; r--) {
		for (int c = 0; c < img.cols(); c++) {
			index = is.get();
			img(r, c)[2] = ctable[index][0];  //B G R pad
			img(r, c)[1] = ctable[index][1];
			img(r, c)[0] = ctable[index][2];

		}
		if (!is)
			return false;
		for (int j = 0; j < padsize; j++) {
			is.get();
		}
	}
	return true;

}

bool load_bmp4(std::istream& is, mat<rgb>& img) {
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
	if (bitppx != 4) {
		error("uncorrect number of bits per pixel");
	}
	uint32_t method, img_size, hres, vres, num_colors, dummy;
	rawread(is, method);

	//discard 12 byte
	rawread(is, img_size);
	rawread(is, hres);
	rawread(is, vres);

	rawread(is, num_colors);
	if (num_colors == 0) {
		num_colors = pow(2, bitppx);
	}

	rawread(is, dummy);
	//read table 
	using quad = array<uint8_t, 4>;
	std::vector<quad> ctable;

	quad pxl;
	for (int i = 0; i < num_colors; i++) {
		pxl[0] = is.get();
		pxl[1] = is.get();
		pxl[2] = is.get();
		pxl[3] = is.get();
		ctable.push_back(pxl);
	}

	// 1)exctract reversed img
	int remaining = (img.cols()/2) % 4; // 4 bit per element sono 1/2 byte
	int padsize = 0;
	if (remaining > 0) {
		padsize = 4 - remaining;
	}
	bitreader br(is);
	uint8_t index;
	for (int r = img.rows() - 1; r >= 0; r--) {
		for (int c = 0; c < img.cols(); c++) {
			index = br(4);
			img(r, c)[2] = ctable[index][0];  //B G R pad
			img(r, c)[1] = ctable[index][1];
			img(r, c)[0] = ctable[index][2];

		}
		if (!is)
			return false;
		for (int j = 0; j < padsize; j++) {
			//is.get();
			br(4);
		}
	}
	return true;

}

bool load_bmp1(std::istream& is, mat<rgb>& img) {
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
	if (bitppx != 1) {
		error("uncorrect number of bits per pixel");
	}
	uint32_t method, img_size, hres, vres, num_colors, dummy;
	rawread(is, method);

	//discard 12 byte
	rawread(is, img_size);
	rawread(is, hres);
	rawread(is, vres);

	rawread(is, num_colors);
	if (num_colors == 0) {
		num_colors = pow(2, bitppx);
	}

	rawread(is, dummy);
	//read table 
	using quad = array<uint8_t, 4>;
	std::vector<quad> ctable;

	quad pxl;
	for (int i = 0; i < num_colors; i++) {
		pxl[0] = is.get();
		pxl[1] = is.get();
		pxl[2] = is.get();
		pxl[3] = is.get();
		ctable.push_back(pxl);
	}

	//padding dei bit
	uint8_t bit_xrow = bitppx * img.cols();
	uint8_t remain_bit = bit_xrow % 8; //  \in (0, 8)-> needed 1 byte more to contain it
	uint8_t byte_xrow = bit_xrow / 8;
	if (remain_bit > 0) {
		++byte_xrow; 
	}

	// 1)exctract reversed img
	int remaining_byte = (byte_xrow) % 4; 
	int padsize = 0;
	if (remaining_byte > 0) {
		padsize = 4 - remaining_byte;
	}
	bitreader br(is);
	uint8_t index;
	for (int r = img.rows() - 1; r >= 0; r--) {
		for (int c = 0; c < img.cols(); c++) {
			index = br(1);
			img(r, c)[2] = ctable[index][0];  //B G R pad
			img(r, c)[1] = ctable[index][1];
			img(r, c)[0] = ctable[index][2];

		}
		//leggo i remaining bit del byte prima di paddare i byte rimanenti
		if (!is)
			return false;
		if (remain_bit > 0) {
			br(8 - remain_bit);
		}

		for (int j = 0; j < padsize; j++) {
			is.get();
			//br(1);
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
	//if (!load_bmp24(is, img)) {
	//	error("impossible to load BMP file");
	//}

	//if (!load_bmp8(is, img)) {
	//	error("impossible to load BMP file");
	//}

	//if (!load_bmp4(is, img)) {
	//	error("impossible to load BMP file");
	//}

	if (!load_bmp1(is, img)) {
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


