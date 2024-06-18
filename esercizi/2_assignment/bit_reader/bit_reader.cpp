#include <fstream>
#include <iterator>
#include <vector>
#include <cstdint>

//template e istream raw_write 
template <typename T> //template per leggere un valore di tipo T nel file
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}
class bitreader
{
	//attributi globali della classe
	uint8_t buffer_;
	int n_ = 0;
	std::istream& is_;


public:
	bitreader(std::istream& is) : is_(is) {}

	uint32_t read_bit() {
		//todo...
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8; // dopo la lettura il n° di bit nel buffer è 8
		}
		--n_;
		return (buffer_ >> n_) & 1;
		//Il bit corrente è ottenuto spostando a destra (>>) il contenuto di buffer_ di n_ posizioni. 
		//Questo porta il bit che si vuole leggere nella posizione meno significativa (LSB).
		// leggo dal bit più significativo al meno significativo 
		// (dall'8° al 1°)

	}

	uint32_t read(uint8_t nbits) {
		uint32_t u = 0;
		while (nbits-- > 0) {
			u = (u << 1) | read_bit();
		 //read bit salva dal piu significativo al meno
		 //il primo bit aggiunto a u sarà il bit più significativo
		}
		return u;
	}

	uint32_t operator()(uint8_t nbits) {
		return read(nbits);
	}

	bool fail() const {
		return is_.fail();
	}

	~bitreader()
	{
	}
};

int main(int argc, char* argv[])
{
	if (argc != 3) {
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return 2;
	}

	bitreader br(is);

	std::vector<int32_t> v;
	//read type in 11 bit
	while (1) {
		int32_t num = br(11);
		if (br.fail()) {
			break;
		}
		//gestione del complemento a due in lettura
		if (num >= 1024) {
			num -= 2048; //se num = 1024-> 1024-2048 = -1024
		}
		v.push_back(num);
	}

	std::ofstream os(argv[2]);
	if (!os) {
		return 3;
	}

	copy(v.begin(), v.end(),
		std::ostream_iterator<int32_t>(os, "\n"));

	return 0;
}