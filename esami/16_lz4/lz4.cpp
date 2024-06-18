#include <fstream>
#include <cstdint>
#include <iostream>
#include <vector>

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

bool lz4decomp(std::istream& is, std::ostream& os) {

	//leggo i primi 4 byte (32 bit) in hex 03 21 4C 18 in litte
	// mdo più diretto
	//uint32_t bytes = 0;
	//raw_read legge i byte in little endian quindi il controllo lo faccio scrivendo il magic number in little endian
	/*raw_read(is, bytes); 
	if (bytes != 0x184c2103) {
		std::cerr << "the magic is not 0x03214C18 \n";
		return false;
	}
*/  //oppure 
	std::vector<uint8_t> bytes(4); // cosi non devi gestire il little endian come nel caso di salvare i 4 byte in un uint32
	for (auto& byte : bytes) {
		raw_read(is, byte);
	}
	if (bytes[0] != 0x03 || bytes[1] != 0x21 || bytes[2] != 0x4c || bytes[3] != 0x18)
	{
		return false;
	}
	//unsigned 32-bit integer in little endian that indicates the length of the uncompressed file
	uint32_t out_len = 0;
	// siccome è little endian se uso la raw read me lo salva in big endian perch il primo B diventa il meno significativo(il primo letto)
	raw_read(is, out_len); 

	//constant value (in hex 00 00 00 4D) in little endian
	uint32_t c_bytes = 0;
	raw_read(is, c_bytes);
	if (c_bytes	!= 0x4d000000)
	{
		return false;
	}
	//la lunghezzad el file di output è file_len 
	std::vector <uint8_t> decoded_f(out_len);
	//ora mi leggo i vari blocks uno per uno e man mano li scrivo dal vector al file os
	//blocks of compressed data
	uint8_t token = 0;
	uint8_t literal_len = 0;
	uint8_t match_len = 0;
	uint8_t k = 0; 
	uint32_t block_len = 0;
	//leggo tutti i blocchi
	while (k<out_len)
	{
		//leggo lugnhezza del blocco e ne estraggo i dati
		raw_read(is, block_len);
		size_t n_bytes = 0;
		//n_bytes che indica quanti bytes ho letto di quelli specificati
		while (n_bytes < block_len) {
			//gestisco la sequenza di lz4 
			raw_read(is, token);
			n_bytes++;
			literal_len |= (0xF0 & token) << 4;
			match_len |= (0x0F & token);
			if (literal_len == 15) {
				//leggo il prossimo byte che mi dice quanto va esteso literal lenght
				uint8_t extend = 0; 
				while (1)
				{
					raw_read(is, extend);
					n_bytes++;
					literal_len += extend;
					if (extend != 255) break;
				}
			}
			//si sono ora literal_len literlals to be read it.
			for (size_t i = 0; i < literal_len; i++)
			{
				uint8_t lit = 0;
				raw_read(is, lit);
				decoded_f.push_back(lit);

			}
			/*if (n_bytes <)
			{

			}*/

		}



	}

	

	return true;
}

int main(int argc, char* argv[]) {

	if (argc != 3)
	{
		return 1;
	}
	std::ifstream is(argv[1], std::ios::binary);
	if (!is)
	{
		return 1;
	}

	std::ofstream os(argv[2], std::ios::binary);
	if (!os)
	{
		return 1;
	}

	lz4decomp(is, os);
	//



	return 0;
}