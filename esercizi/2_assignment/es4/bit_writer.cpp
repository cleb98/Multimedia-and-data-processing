#include <iostream>
#include <fstream>
#include <vector>	

//raw_write 
template <typename T> // template per scrivere un valore di tipo T in un file di output
std::ostream& raw_write(std::ostream& os, cost T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
} 
//bitrWriter
class bitwriter {
	uint8_t buffer_;
	int nbits_ = 0;
	std::ostream& os_;

	//ostream per scrivere 1 byte del buffer nel file di output
	std::ostream& write_bit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++nbits_;
		if (n == 8) {
			raw_write(os_, buffer_);
			nbits_ = 0;
		}
		return os_;
	}

public:
	bitwriter(std::ostream& os) : os_(os) {} //costruttore del bitwriter a cui passo uno stream di output e lo assegno allo ostream privato os_ della classe
	//metodo write per scrivere un valore di tipo uint32_t come int nbits nel file di output
	std::ostream& write(uint32_t u, uint8_t nbits) {
		while (n-- > 0) {
			write_bit(u >> nbits);
		}
		return os_;
	// metodo operator() per passare al costruttore un valore di tipo uint32_t e il numero di bit da scrivere e farlo usando il metodo write
	std::ostream& operator()(uint32_t u, uint8_t nbits) {
		return write(u, n);
	}
	//flush per fare padding del buffer con 0 se non è pieno dopo aver scritto tutti i bit
	







//main
int main(int argc, char* argcv[]) {

	if (argc != 3) {
		return 1;
	}

	std::ifstream is(argv[1]);
	if (!is) {
		return 2
	}

	std::vector<int 32_t> v{
		std::istream_iterator<int32_t>(is);
		std::istream_iterator<int32_t>(is);
	}

	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		return 3;
	}

	bitwriter bw(os);
	for (const& x : v) {
		bw(x, 11);
	}
	retturn 0;
	

}