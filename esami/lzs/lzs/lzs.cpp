#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include <cstdint>


void error(const std::string& s) {
	using namespace std;
	cout << s;
	exit(EXIT_FAILURE);
}

template <typename T>
std::istream& raw_read(std::istream& is, T& val) {
	return is.read(reinterpret_cast<char*>(&val), sizeof(T));
}

template <typename T>
std::ostream& raw_write(std::ostream& os, T& val) {
	return os.write(reinterpret_cast<char*>(&val), sizeof(T));
}

class bitreader {

	uint8_t buffer_ = 0;
	uint8_t n_ = 0;
	std::istream& is_;

	uint64_t readbit() {
		if (n_ == 0) {
			raw_read(is_, buffer_); //legge sizeof(buffer_) bit da is_ e li salva in buffer_
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

public:

	bitreader(std::istream& is) :is_(is) {};
	~bitreader() {};

	uint64_t operator()(size_t bit_to_read) {
		uint64_t byte = 0;
		while (bit_to_read-- > 0) {  //bitreader(11) si muove sull'istream leggendo 11 bit
			byte = byte | (readbit() << bit_to_read);
			//read di readbit();
		}
		return byte;
	}

	bool fail() {
		return is_.fail();
	}
};

void lzs_decompress(std::istream& is, std::ostream& os) {

	std::vector<char> buffer;
	bitreader br(is);
	uint16_t offset = 0;
	uint16_t len = 0;
	char c = ' ';
	std::string s;
	//leggo tutto il file per decodificare 
	while (true) {
		if (!is)
			break;

		//leggo il primo bit per capire se è 0 o 1, se 0 faccio copy, senno la run
		uint8_t bit = br(1);
		if (bit == 0) {
			//copy, leggo 1B lo scrivo in out, aggiorno il buffer dict
			c = br(8); 
			raw_write(os, c);

			if (buffer.size() < 2048) {
				buffer.insert(buffer.begin(), c);
			}
			else {
				buffer.pop_back();
				buffer.insert(buffer.begin(), c);
			}			

		}
		else if (bit == 1) {
			//leggo offset		// 1 o 0
			if (br(1) == 1) {
				offset = br(7);
				if (offset == 0b0000000) //EOF
					break;
			}
			else				//br(1) == 1
				offset = br(11);
			//leght
			// leggo primo bit di lenght
			if (br(1) == 0) { 
				len = br(1) + 2; //br(1) è 0 o 1 e devo dec 2 o 3
			}
			//se son qua primo bit di len è 1
			else if (br(1) == 0) { // 10 -> 4 
				len = 4;
			}
			//ho gia letto due bit arrivato qui e sono 11
			else if (br(1) == 0) {// 110
				len = br(1) + 5; // br1 = 0 ->len = 5,=1 -> 6
			}
			else if (br(1) == 0) { // 111 se 0 è 7
				len = 7;
			}
			else if (br(1) == 1) { //1111 -> 
				//ne leggo 4 e vedo che devo fa
				int num = br(4);
				int N = 1; // N = len +7 /15 -> se so N so anche len
				while (num == 0b1111)
				{
					++N; 
					num = br(4);
				}
				// so quanto vale N e decodifico len e leggo ibit rimanenti;
				br(4);
				len = N * 15 - 7;
			}
			//ora so offset e len
			if (len <= offset) {
				for (int i = 0; i < len; i++) {
					c = buffer[offset - 1 - i];
					raw_write(os, c); //buff[0] è ultimo el buf[2047] il primo, vado in buff[offset -1]
					s += c;
				}
			}
			else //len > offset 
			{  //len - off = k  -> len = offset + k 
				for (int i = 0; i < offset; i++) { //copio offset elementi dal buff
					c = buffer[offset - 1 - i];
					raw_write(os, c);
					s += c;
				}
				int k = len - offset;
				//ricomincio da buffer[offset - 1] e ho k elementi della stringa(lunga offset) 
				// prob è k > offset
				auto it = *s.begin();
				int count = 0;
				while (k --> 0) //k = 4 
				{
					raw_write(os, it);
					count++;
					s += it;
					//se siamo a s.end allora non incrementare
					if (count == offset) {
						it = *s.begin();
						count = 0;
					}
					else ++it;
				}
			}
			for (char ch : s) {
				if (buffer.size() < 2048) {
					buffer.insert(buffer.begin(), ch);
				}
				else {
					buffer.pop_back();
					buffer.insert(buffer.begin(), ch);
				}
			}				
			s.clear(); //pulisco la stringa da mettere nel buffer
		}
		else 
			error("impossible to read a bit on input file");


	}
}


int main(int argc, char* argv[]) {

	std::string fin = argv[1];
	std::string fout = argv[2];

	std::ifstream is(fin, std::ios::binary);
	if (!is)
		error("impossible to read" + fin);
	std::ofstream os(fout);
	if (!os)
		error("impossible to write" + fout);
	//bitreader br(is);
	//uint8_t byte = br(8);
	lzs_decompress(is, os);
	
	return EXIT_SUCCESS;
}