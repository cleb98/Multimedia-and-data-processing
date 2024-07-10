#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <algorithm>
#include <utility>
#include <sstream>
#include <cstdint>
#include <cmath>


using rgb = std::array<uint8_t, 3>;

template <typename T>
class mat {
	int rows_, cols_;
	std::vector<T> data_;
	
public:
	mat(int rows = 0, int cols = 0) : 
		rows_(rows), cols_(cols), data_(rows* cols) {};

	~mat() {};

	auto& operator() (int rows, int cols) {
		return data_[rows * cols_ + cols];
	}

	//in this way you cant modify data_ using [] you need auto&
	//auto operator[] (int i) const{
	//	return data_[i];
	//}

	auto& operator[] (int i) {
		return data_[i];
	}

	int columns() const{
		return cols_;
	}

	auto& data() {
		return data_;
	}

	const auto& data() const {
		return data_;
	}

	int rows() const {
		return rows_;
	}

	int size() const {
		return data_.size();
	}

	void resize(int rows, int cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols);
	}

	//char* raw_data() const {
	//	return reinterpret_cast<char*> (data_.data()); //data_.data() or &data_[0]
	//}
	const char* raw_data() const {
		return reinterpret_cast<const char*> (data_.data()); //data_.data() or &data_[0]
	}

	int raw_size() const {
		return rows_ * cols_ * sizeof(T);
	}

	auto begin() const {
		return data_.begin();
	}

	auto end() const {
		return data_.end();
	}
	
};

void error(const std::string& s) {
	std::cout << s;
	exit(EXIT_FAILURE);
}

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

template <typename T>
/*read max 8 byte*/
void read_be(std::istream& is, T& bytes, size_t size = sizeof(T)) {
	bytes = 0;
	while (size-- > 0) {
		bytes = (bytes << 8) + is.get();
	}
}

std::pair<int, int> read_resolution_string(std::istream& is) {
	using namespace std;
	int rows = 0;
	int cols = 0;
	string token(2, ' ');
	string w, h;
	raw_read(is, token[0], 2);
	if (token != "-Y") {
		error("error in resolution string");
	}
	is.get();
	raw_read(is, h[0], 4);
	token.clear();
	token.assign(2, ' ');
	is.get();
	raw_read(is, token[0], 2);
	if (token != "+X") {
		error("error in resolution string");
	}
	is.get();
	raw_read(is, w[0], 4);
	is.get();//legge a capo finale
	cols = stoi(w);
	rows = stoi(h);
	pair<int, int> H_W = { rows, cols };
	return  H_W;

}

float decode_fp(uint8_t x, uint8_t e) {
	float xf = (static_cast<float>(x) + 0.5) * pow(2, static_cast<float>(e) - 128) / 256;
	return xf;
}

uint8_t decode_px(float x, float min, float max) {
	float p = 255 * pow((x - min) / (max - min), 0.45);
	return static_cast<uint8_t>(p);
}


bool load_hdr(std::istream& is, mat<rgb>& img) {
	using namespace std;
	string magic(11, ' ');
	//getline(is, magic);
	raw_read(is, magic[0], 11);
	if (magic != "#?RADIANCE\n") {
		error("wrong magic word");
	}

	string line;
	while (true) {
		getline(is, line);
		if (line.empty()) { //"" is empty string
			break;
		}
		size_t pos = line.find('=');
		if (line.substr(0, pos) == "FORMAT") {
			if (line.substr(pos + 1) != "32-bit_rle_rgbe")
				error("FORMAT != 32-bit_rle_rgbe");
		}
	}


	/*1*//*
	getline(is, line);
	istringstream ss(line);
	string token;
	ss >> token;
	if (token != "-Y") {
		error("error in resolution string");
	}
	ss >> rows;

	ss >> token;
	if (token != "+X") {
		error("error in resolution string");
	}
	ss >> cols;
	*/
	/*2 -> read_resolution_string */
	auto rowscols = read_resolution_string(is);
	img.resize(rowscols.first, rowscols.second);


	//fill the img
	std::vector<uint8_t> red;
	std::vector<uint8_t> green;
	std::vector<uint8_t> blu;
	std::vector<uint8_t> exp;
	for (int i = 0; i < rowscols.first; i++) {
		if (is.get() != 2) {
			error("data enconding is wrong");
		}
		if (is.get() != 2) {
			error("data enconding is wrong");
		}
		uint16_t nc;//num cols
		read_be(is, nc);
		for (int a = 0; a < 4; a++) {
			std::vector<uint8_t>* v = nullptr;
			if (a == 0) {
				v = &red;
			}
			else if (a == 1) {
				v = &green;
			}
			else if (a == 2) {
				v = &blu;
			}
			else {
				v = &exp;
			}
			int count = 0;
			while (count < nc) {//read cols r
				uint8_t L = is.get();
				if (L < 128) {
					for (int i = 0; i < L; i++) {
						v->push_back(is.get());
						count++;
					}
				}
				else {
					uint8_t val = is.get();
					for (int i = 0; i < (L - 128); i++) {
						v->push_back(val);
						count++;
					}
				}
			}
		}
	}
	vector<float> r;
	vector<float> g;
	vector<float> b; 
	float ri, gi, bi;
	for (size_t s = 0; s < img.size(); s++){
		ri = decode_fp(red[s], exp[s]);
		gi = decode_fp(green[s], exp[s]);
		bi = decode_fp(blu[s], exp[s]);
		r.push_back(ri);
		g.push_back(gi);
		b.push_back(bi);
	}
	//min di ogni vector e min glob
	auto r_minmax = std::minmax_element(r.begin(), r.end());
	auto g_minmax = std::minmax_element(g.begin(), g.end());
	auto b_minmax = std::minmax_element(b.begin(), b.end());
	
	
	float rmin, gmin, bmin, rmax, gmax, bmax;
	rmin = static_cast<float>(*r_minmax.first);
	gmin = static_cast<float>(*g_minmax.first);
	bmin = static_cast<float>(*b_minmax.first);
	rmax = static_cast<float>(*r_minmax.second);
	gmax = static_cast<float>(*g_minmax.second);
	bmax = static_cast<float>(*b_minmax.second);

	float min_g = min({rmin, gmin, bmin});
	float max_g = max({rmax, gmax, bmax});

	for (int i = 0; i < img.size(); i++) {
		red[i] = decode_px(r[i], min_g, max_g);
		green[i] = decode_px(g[i], min_g, max_g);
		blu[i] = decode_px(b[i], min_g, max_g);
	}

	for (int i = 0; i < img.size(); i++) {
		img[i] = { red[i], green[i], blu[i] };
	}
	/*______*/
	return 1;
}

bool save_pam(std::ostream& os, mat<rgb>& img) {
	os << "P7\n";
	os << "WIDTH " << img.columns() << "\n";
	os << "HEIGHT " << img.rows() << "\n";
	os << "DEPTH 3\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE RGB\n";
	os << "ENDHDR\n";
	//os.write(img.raw_data(), img.raw_size());
	for (int r = 0; r < img.rows(); r++) {
		for (int c = 0; c < img.columns(); c++) {
			os.put(img(r, c)[0]);
			os.put(img(r, c)[1]);
			os.put(img(r, c)[2]);
		}
	}
	return 1;
}

int main(int argc, char* argv[]) {

	if (argc != 3) {
		error("syntax error:\n hdr_decode <input file .HDR> <output file .PAM>");
	}
	
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		error("impossible to open " + std::string(argv[1]));
	}
	mat<rgb> img;
	if (!load_hdr(is, img)) {
		error("impossible to load hdr image");
	}
	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		error("impossible to open " + std::string(argv[2]));
	}
	
	if (!save_pam(os, img)) {
		error("impossible to save image.pam");
	}
	/*_________________*/
	return EXIT_SUCCESS;
}
