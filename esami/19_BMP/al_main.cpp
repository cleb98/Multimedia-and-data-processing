#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>

using rgb = std::array<uint8_t, 3>;

template <typename T>
struct mat {
	int rows_;
	int cols_;
	std::vector<T> data_;

	mat(int rows = 0, int cols = 0) :
		rows_(rows), cols_(cols), data_(rows* cols) { }

	int rows() const { return rows_; }
	int cols() const { return cols_; }

	T& operator()(int r, int c) {
		return data_[r * cols_ + c];
	}
	const T& operator()(int r, int c) const {
		return data_[r * cols_ + c];
	}

	void resize(int rows, int cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols);
	}
};

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

bool load_bmp(std::istream& is, mat<rgb>& img) {
	using namespace std;

	string magic(2, ' ');
	is.read(reinterpret_cast<char*>(&magic[0]), 2);
	if (magic != "BM") {
		cout << "Wrong magic\n";
		return false;
	}

	uint32_t filesize;
	raw_read(is, filesize);

	for (int i = 0; i < 4; i++) { // discard bits 6-10
		is.get();
	}

	uint32_t data_offset;
	raw_read(is, data_offset);

	uint32_t header_size;
	raw_read(is, header_size);
	if (header_size != 40) {
		cout << "Wrong header size\n";
		return false;
	}

	int32_t width, height;
	raw_read(is, width);
	raw_read(is, height);
	img.resize(height, width);

	uint16_t color_planes;
	raw_read(is, color_planes);
	if (color_planes != 1) {
		cout << "Wrong color planes number\n";
		return false;
	}

	uint16_t bpp;
	raw_read(is, bpp);

	uint32_t compr_method;
	raw_read(is, compr_method);
	if (compr_method != 0) {
		cout << "Compression method not allowed\n";
		return false;
	}

	for (int i = 0; i < 12; i++) { // discard 12 bytes
		is.get();
	}

	uint32_t num_colors;
	raw_read(is, num_colors);
	if (num_colors == 0) {
		num_colors = uint64_t(1) << bpp;
	}

	for (int i = 0; i < 4; i++) { // discard 4 bytes
		is.get();
	}

	// a row is composed of cols * 3 bytes
	int remaining = (img.cols() * 3) % 4;
	int padsize = 0;
	if (remaining > 0) {
		padsize = 4 - remaining;
	}

	for (int r = img.rows() - 1; r >= 0; r--) {
		for (int c = 0; c < img.cols(); c++) {
			// bitmap triples are BGR
			img(r, c)[2] = is.get();
			img(r, c)[1] = is.get();
			img(r, c)[0] = is.get();
		}

		for (int i = 0; i < padsize; i++) { // discard padding
			is.get();
		}
	}

	return true;
}

bool save_pam(std::ostream& os, const mat<rgb>& img) {
	using namespace std;

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

int main(int argc, char* argv[]) {
	using namespace std;
	if (argc != 3) {
		cout << "Wrong argument count\n";
		return 1;
	}

	ifstream is(argv[1], ios::binary);
	if (!is) {
		cout << "Unable to open input file\n";
		return 1;
	}

	ofstream os(argv[2], ios::binary);
	if (!os) {
		cout << "Unable to open output file\n";
		return 1;
	}

	mat<rgb> img;

	if (!load_bmp(is, img)) {
		cout << "BMP loading failed\n";
		return 1;
	}

	if (!save_pam(os, img)) {
		cout << "Unable to save image as PAM\n";
		return 1;
	}

	return 0;
} 