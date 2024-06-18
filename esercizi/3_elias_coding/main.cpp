#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <stdlib.h>     /* exit, EXIT_FAILURE */ //forse non serve a niente
#include <string>
#include <fstream>
//#include <bit> //not available in c++ < 20
#include <cmath>
#include <iterator>



void syntax() {
	std::cout << "Syntax:\n"
		"error in the argv "
		"elias [c|d] <filein> <fileout>\n";
	exit(EXIT_FAILURE);
				
}
void error(const std::string& s) {
	std::cout << s;
	exit(EXIT_FAILURE);
}

class bitwriter {
	std::ostream& os_;
	uint8_t buffer_ = 0;
	uint8_t nbit_ = 0;
	
	//scrivi bit nel buffer
	void write_bit(uint64_t bit) {
		buffer_ = (buffer_ << 1) | (1 & bit);
		nbit_++;
		if (nbit_ == 8) {
			os_.put(buffer_);
			nbit_ = 0;
		}


	}
public:
	bitwriter(std::ostream & os) : os_(os) {}
	~bitwriter() {
		flush();
	}
	std::ostream& write(uint64_t x, uint64_t bit_of_x) {
		while (bit_of_x -- > 0) {
			write_bit(x >> bit_of_x);
		}
		return os_;
	}

	std::ostream& flush(uint64_t bit = 0) {
		while (nbit_ > 0)
			//metto zeri quanti nbit davanti al buffer
			write_bit(bit);
		return os_;
	}

};


class elias_writer {
	bitwriter bw_;
	//mapping
	//
	static uint64_t map(int x) {

		if (x < 0)
			return -2 * x;
		else
			return 2 * x + 1;


	}
	//std::bit_width non va
	static uint64_t num_bit(int x) {
		uint64_t bits = (static_cast<uint64_t> (std::floor(log2(x)))) *2 +1;
		return bits;

	}

public:
	//constructor -> inzializzo gli attributi di classe
	elias_writer(std::ostream& os) : bw_(os) {}
	void write(uint64_t x) {
		uint64_t x_map = map(x);
		uint64_t n_bit = num_bit(x_map);
		//bitwrite mper mappare il numero ottenuto al suo contenuto di bit 
		bw_.write(x_map, n_bit);
	}
};

void compress(	
	const std::string& fin, 
	const std::string& fout
) 
{
	std::ifstream is(fin);
	if (!is) {
		error("impossiile aprire " + fin + " in lettura");
	}
	std::ofstream os(fout, std::ios::binary);
	if (!os) {
		error("impossiile aprire" + fout + "in scrittura");
	}
	elias_writer ew(os);
	//itero su ifstream con iterator per leggere numeri
	// uso elias_writer::write per mappare i numeri al loro elias coding
	std::vector<int32_t> v{
		std::istream_iterator<int32_t>(is), 
		std::istream_iterator<int32_t>()
	};
	for (auto& c : v) {
		ew.write(c);
	}
}


class bitreader {
	uint8_t buffer_;
	uint8_t nbit_ = 0;
	std::istream& is_;

//per readbit()
//fillo il buffer e setto nbits_ = dim buffer 
// devo shiftare il buffer di nbits_ -1 per leggere il bit + significativo infine decremento il numero bit per leggere il secondo piu significaitbo
// e cosi via fino al meno significativo
// nbit_ = 4 
// buffer_ = 1100 
// nbit_ = 3
// buffer_ << nbit = 0001 & 1
// nbit = 2 
//buffer_ << nbit = 0011 & 1
// nbit = 1
// buffer_ << nbit = 0110 & 1
// passo l'if e poi decremento nbit a zero e leggo il meno significativo
// al prossimo giro entro nell'if e rifillo il buffer

	uint64_t readbit() {
		if (nbit_ == 0) {
			buffer_ = is_.get();
			nbit_ = 8;
		}
		--nbit_;
		return (buffer_ >> nbit_) & 1;
	}


public:
	//costruttore
	bitreader(std::istream& is) : is_(is) {};
	uint64_t operator ()(uint64_t bit_to_read) {
		uint64_t val = 0; //ricordiamoci che non so con quanti bit è rappresentato il val finale, quello lo capisce l'elias_reader dal numero dizeri prima degli uno
		while (bit_to_read -- > 0) {
			// leggo b1 val = 0000 + b1 = 000b1, leggo b2 val = val << 1 + b2 = 00b10 + b2 =  
			val = (val << 1) | readbit();
		}
		return val;

	}


};

class elias_reader {
	bitreader br_;
	//leggo valore a quanti bit? anche piu di 8 quindi uint64 pero c'è il segno quindi forse serve int32t
	static int32_t unmap(uint64_t x) {

		if (x % 2 == 0)
			return static_cast<int32_t>(x) / -2;
		else
			return (static_cast<int32_t>(x) - 1) / 2;
	}


public:
	elias_reader(std::istream& is) : br_(is) {};
	//prendo un certo numero di byte (64/8) e li leggo bit per bit cosi da decodificare il valore da demappare
	int32_t read() {
		uint64_t val = 0; // valore da rimappare
		uint64_t bit = 0; //bit che leggo
		uint64_t n = 0; //contatore degli zeri

		//capire come implementare che devo leggere bit by bit fino al primo 1, e contando gli zeri che ci sono prima capisco il valore da decodificare
		while (true) { 
			bit = br_(1);
			// 00100 | 0 1, 0 2, 1 add 1, 
			if (bit == 0)
				n++;
			else {
				val = (val | bit); //001 -> 100
				val = (val << n) | br_(n); 
				break;
			}
				


		}
		int32_t unmapped = unmap(val);
		return unmapped;
		
	}


};

void decompress(
	//passo i file da leggere come prima
	const std::string& fin,
	const std::string& fout
) {
	std::ifstream is(fin, std::ios::binary);
	if (!is) error("impossiile aprire " + fin + " in lettura in binary mode");

	std::ofstream os(fout);
	if (!os) error("impossiile aprire" + fout + "in scrittura in txt mode");

	//istnzio elias_reader
	elias_reader er(is);
	//vado a leggere su input stream un val alla volta(elias reader sa quanti bit leggere e come fare l'unmpa del valore)
	//controllo se l'iftream è finito semai rompo il while
	//metto in os il val letto con lo stream
	while (true)
	{
		int32_t val = er.read();
		if (!is) break;
		os << val << "\n";
	}
	
};






int main(int argc, char* argv[])
{
	if (argc != 4) {
		syntax();
		
	}


	std::string command = argv[1];
	if (command == "c") {
		//std::cout << "compression\n";
		compress(argv[2], argv[3]);
	}
	else if (command == "d")
	{
		//std::cout << "decompression";
		decompress(argv[2], argv[3]);

	}
	else
	{
		std::cout << "no argv";
	}
	return

}
