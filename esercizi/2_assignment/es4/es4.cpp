#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>



// classe BitWriter

template <typename T>
//implementazione ostream per scrivere un valore di tipo T in un file di output
std::ostream &raw_write(std::ostream &os, const T& value, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*> (&value), size);
}

class bitwriter {
	//definisco le variabili private della classe bitwriter
	uint8_t buffer_; // buffer per scrivere 8 bit alla volta (1 byte)
	int nbits_ = 0; // conta numero di bit scritti nel buffer
	std::ostream& os_; // stream di output su cui scrivere i bit del buffer

	//implementazione ostream per scrivere 8 bit alla volta nel buffer
	//bit (intero di 32 bit) lo scrivo nel buffer di 8 bit
	std::ostream& writebit(uint32_t bit) { 
		//step per scrivere un bit nel buffer:
		// shifta il buffer di 1 bit a sinistra, (es: 0001 << 1 = 0010)
		// AND bit con 1 per ottenere il bit meno significativo (es: [bit(= 0000 o 0001)] & 1 = [0 o 1] )
		// OR bit con il buffer per aggiungere il bit al buffer (es: [buffer(= 0010)] | [bit(=0001)] = 0011 )
		buffer_ = (buffer_ << 1) | (bit & 1);
		++nbits_; // conto il numero di bit scritti nel buffer (max 8)
		if (nbits_ == 8) { // se il buffer è pieno
			raw_write(os_, buffer_); // scrivo il buffer di 8 bit (il byte) nel file di output
			nbits_ = 0; // resetto il numero di bit scritti nel buffer
		}
		return os_;
	}

public:
	//costruttore della classe bitwriter
	bitwriter(std::ostream& os) : os_(os) {} // prende in input uno stream di output e lo assegna a os_

	//funzione per scrivere un valore di tipo uint32_t nel file di output
	std::ostream& write(uint32_t value, int nbits) {
		//while(nbits-->0) {} // equivalente al for sopra
		for (int i = nbits - 1; i >= 0; --i)
			writebit(value >> i); // value è il bit da scrivere che viene shiftato di i bit a destra per renderlo il bit meno significativo (aka il bit da scrivere)
		return os_;
	}
	//overload dell'operatore () per passare a bitwriter un valore di tipo uint32_t e il numero di bit da scrivere
	std::ostream& operator()(uint32_t value, int nbits) {
		return write(value, nbits);
	}
	//flush: serve per fare padding del buffer con 0 se non è pieno dopo aver scritto tutti i bit
	std::ostream& flush(uint32_t bit = 0) {
		while(nbits_ > 0) // finchè il buffer non è pieno
			writebit(bit); // scrivo 0 nel buffer
		return os_;
	}

	//serve condizione per usare il flush
	~bitwriter() { // se alla distruzione dell'oggetto il buffer non è vuoto
		flush(); // faccio padding del buffer con 0, nella writebit il nbits_ viene resettato a 0
	}
};






// write_int11 <filein.txt> <fileout.bin>
int write_int11(const char* in, const char* out) {
	std::ifstream is(in);
	if (!is) {
		std::cout << "Error opening file " << std::endl;
		return 2;
	}
	//questo modo di leggere un file è obsoleto, meglio usare gli stream iterators come sotto
	/*int32_t num = 0;
	std::vector<int32_t> v;
	while (1) {
		is >> num;
		if (!is) break;
		v.push_back(num);
	}
	*/
	//questo è il modo corretto di leggere un file
	std::vector<int32_t> v{
		std::istream_iterator<int32_t>(is),
		std::istream_iterator<int32_t>()
	};
	//apro il file di output
	std::ofstream os(out, std::ios::binary);
	if (!os) {
		std::cout << "Error opening file " << std::endl;
		return 3;
	}
	//uso la classe bitwriter per scrivere i bit nel file di output
	bitwriter bw(os);
	//scrivo nel file di output il numero di elementi del vettore
	for (auto& value : v) {
		bw(value, 11);
	}
	return 0;

}






int main(int argc, char* argv[]) {
	
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " <filein.txt> <fileout.bin>" << std::endl;
		return 1;
	}

	int out = write_int11(argv[1], argv[2]);

	
}