#include <cstdint>
#include <string>
#include <array>
#include <iostream>
#include <fstream>
#include <vector>
#include  <unordered_map>
#include <limits>
#include <cmath>

template <typename T>
std::istream& raw_read(std::istream& is, T& v, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&v), size);
}


using rgb = std::array<uint8_t, 3>;

template <typename T>
class mat {
	int rows_, cols_;
	std::vector<T> data_;

public:
	mat(int r = 0, int c = 0) : cols_(c), rows_(r), data_(r* c) {	}

	int cols() const {
		return cols_;
	}

	int rows() const {
		return rows_;
	}

	T* data() {
		return data_.data();
	}

	int size() const {
		return cols_*rows_;
	}

	int raw_size() const {
		return cols_ * rows_ * sizeof(T);
	}

	char* raw_data() {
		return reinterpret_cast<char*>(&data_[0]);
	}

	void resize(int r, int c) {
		rows_ = r;
		cols_ = c;
		data_.resize(r * c);
	}

	T& operator() (int r, int c) {
		return data_[r * cols_ + c];
	}

	T& operator[] (int i) {
		return data_[i];
	}

	void push_back(T& val) {
		data_.push_back(val);
	}

};

void error(const std::string& s) {
	std::cout << "error: " << s;
	exit(EXIT_FAILURE);
}

struct ppm_header {
	std::string magic;
	int h_ = 0;
	int w_ = 0;
	int maxval_ = 0;
};

bool write_ppm_header(std::ostream& os, ppm_header& h) {
	os << "P6\n";
	os << h.w_ << " ";
	os << h.h_ << "\n";
	os << h.maxval_ << "\n";
	return 1;
}


bool read_ppm_header(std::istream& is_, mat<rgb>& img) {
	//RICORDARSI CHE PER LEGGERE HEADER widht eight and maxval si usano int
	// e si ASEGNA con streamer operator che si mangia lo spazio finale
	ppm_header header;
	header.magic.resize(2);
	std::getline(is_, header.magic);
	if (header.magic != "P6") {
		error("magic word is wrong");
	}
	if (is_.peek() == '#') {
		is_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	is_ >> header.w_;
	is_.get();
	is_ >> header.h_;
	is_.get();
	is_ >> header.maxval_;
	if (header.maxval_ < 0 || header.maxval_ > 65536) {
		error("invald maxval");
	}
	is_.get();
	img.resize(header.h_, header.w_);
	/* (non mi serve leggo 4 byte in big endian tenendone il conto e se il num di byte letti %4 !=0 paddo)
	
	for (int r = 0; r < img.rows(); r++) {
		for (int c = 0; c < img.cols(); c++) {
			img(r, c)[0] = is_.get();
			img(r, c)[1] = is_.get();
			img(r, c)[2] = is_.get();
		}
	} */ 

	return 1;
}


uint64_t read_be(std::istream& is ,int n_byte = 4) {
	uint64_t out = 0;
	while (n_byte-->0)
	{
		out = (out << 8) | is.get();
	}
	return out;
}

struct encode_table {
	std::unordered_map<int, char> t;
	std::string sym = {
		'.',
		'-',
		':',
		'+',
		'=',
		'^',
		'!',
		'/',
		'*',
		'?',
		'&',
		'<',
		'>',
		'(',
		')',
		'[',
		']',
		'{',
		'}',
		'@',
		'%',
		'$',
		'#'
	};

	void build() {
		for (int i = 0; i < 10; i++) {
			t[i] = '0' + i;
		}
		char c = 'a';
		int count = 0; 
		for (int i = 10; i < 36; i ++) {
			t[i] = c + count;
			count++;
		}
		c = 'A';
		count = 0;
		for (int i = 36; i < 62; i++) {
			t[i] = c + count;
			count++;
		}
		for (size_t i = 62; i < 85; i++)
		{
			t[i] = sym[i - 62];
		}
	}

	int find_k_from_v(char v) {
		for (auto it : t) {
			if ((it.second) == v) {
				return it.first;
			}
		}
	}
};

/* write it in debug proerties: c N 2x2red.ppm my_red_N13.txt, N /in [0, +inf[ */
bool encode_z85(std::string& fin, std::string& fout, mat<rgb>& img, int N) {

	std::ifstream is(fin, std::ios::binary);
	if (!is) {
		error("impossible to read input stream (argv[3])");
	}
	std::ofstream os(fout);
	if (!os) {
		error("impossible to write output stream argv[4]");
	}
	read_ppm_header(is, img);
	os << img.cols() << ',' << img.rows() << ',';
//encode img
	//padding handle
	encode_table tab;
	tab.build();
	int rim, pad;
	pad = 0;
	size_t len = img.size() * 3;
	rim = len % 4; //da verificare se la gestione del padding works weel
	if (rim > 0)
		pad = 4 - rim;

	uint64_t val = 0;
	int count_N = 0;
	for (int i = 0; i < len; i += 4) {
		//gestione padding
		if ((len - i) < 4) {//se mancano rim byte da leggere li paddo
			val = read_be(is, rim);
			for (int j = 0; j < pad; j++) {
				val = (val << 8);
			}
		}
		else {
			val = read_be(is);//leggo 4 byte
		}
		std::vector<uint8_t> nums;
		//ora ho val devo trasformrlo in base85 tenendo conto dell'eventuale shift
		int r;
		int times = 5;
		while (times --> 0) {
			r = val % 85;
			val = val / 85;
			nums.insert(nums.begin(), r);
		}
		int index;
		for (auto& it: nums) {
			index = int(it) - (count_N * N);
			int rim = 0;
			if (index < 0) {
				rim = (-index) % 85;
				index = 85 - rim;
			}
			else if(index > 84)
			{
				rim = index % 85;
				index = 85 - rim;
			}

			auto kv = tab.t.find(index); 
			os.put(kv->second);
			count_N++;
		}

	}
	return 1;
}

//template <typename T>
//std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
//	return os.write(reinterpret_cast<char*> (&val), size);
//}

template <typename T>
void write_be(std::ostream& os,const T& bytes, size_t size = sizeof(T)) {
	//premdo byte piu signific e lo scrivo e continuo fino all'ultimo
	while (size --> 0)
	{
		uint8_t byte = (bytes >> (size*8));
		os.put(byte);
	}
}

bool decode_z85(std::string& fin, std::string& fout, int N = 0) {
	std::ifstream is(fin);
	if (!is) {
		error("impossible to read input stream (argv[3])");
	}
	std::ofstream os(fout, std::ios::binary);
	if (!os) {
		error("impossible to write output stream on argv[4]");
	}

	encode_table tab;
	tab.build();
	ppm_header h;
	is >> h.h_; //rows
	is.ignore();
	is >> h.w_; //cols
	is.ignore();
	h.maxval_ = 255;

	write_ppm_header(os, h);
	//img
	std::vector <char> chars(5);
	std::vector <int> indexes;
	int count_N = 0;
	size_t rim = (h.h_*h.w_ * 3) % 4;
	size_t len = (h.h_ * h.w_ * 3 / 4) + rim;

	for (size_t s = 0; s < len; s++)
	{
		if (is.peek() == EOF)
			break;
		for (size_t i = 0; i < chars.size(); i++) {
			int index;
			is >> chars[i];
			index = tab.find_k_from_v(chars[i]);
			//index = index - (count_N * N); -> f inversa
			index += (count_N * N);
			int rim = 0;
			if (index > 84) {
				rim = index % 85;
				index = 85 - rim;
			}
			indexes.push_back(index);
			++count_N;
		}
		//ottenuti i 5 indici 
		//li riconverto da base85 a base10 (int ~4byte), 
		uint32_t n_10 = 0;
		for (int m = 0; m < 5; m++) {
			int i = 4 - m;
			auto a = pow(85, i);
			n_10 += indexes[m] * (pow(85, i));
		}

		indexes.clear();
		if (s - (h.h_*h.w_*3) == rim && rim > 0) {//quando s = len -> s - (h.h_*h.w_*3) == rim in teoria e allora li devo depaddare
			write_be(os, n_10, rim);
		}
		else
			//ho i 4 byte da scrivere in output in big endian;
			write_be(os, n_10);
	}
	
	return 1;
}




int main(int argc, char* argv[]) {
	
	if (argc != 5) {
		error("syntax should be\n Z85rot {c | d} <N> <input file> <output file>");
	}
	std::string command = argv[1];
	std::string N = argv[2];
	std::string fin(argv[3]);
	std::string fout(argv[4]);

	mat<rgb> img;
	
	if (command == "c") {
		std::cout << "compress";
		
		encode_z85(fin, fout, img, stoi(N));
	}
	else {
		std::cout << "decompress";
		decode_z85(fin, fout, stoi(N));
	}

	return EXIT_SUCCESS;
}