#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <iterator>

void error(const std::string& s) {
	std::cout << s;
	exit(EXIT_FAILURE);
}

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	uint8_t buffer_ = 0;
	uint8_t n_ = 0;
	std::istream& is_;
	
	uint8_t readbit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::istream& is) : is_(is) { };
	~bitreader() {};

	/* used for read a specified n° of bits from a istream */
	uint64_t operator() (size_t nbit) {
		uint64_t u = 0;
		while (nbit-- > 0) {
			u = (u << 1) | readbit();
		}
		return u;
	}
};

struct lzvn {
	/*
	 magic | block type
	  bvxn | LZVN compressed
	  bvx$ | end of stream
	*/
	std::string magic_block;
	int32_t out_byte = 0; // number of bytes that the output will require
	int32_t block_size = 0; // number of encoded (source) bytes.
	std::string end_block;
	
	//decoding variable
	uint16_t len = 0;
	uint16_t match_len = 0;
	uint16_t dis = 0;
	//flusso di byte mesi in output
	std::vector<uint8_t> out;
	//lookuptable
	std::map<uint8_t, std::string> h =
	{ { 0b00000000, "sml_d"},
	  { 0b00000001, 	"sml_d" },
	  { 0b00000010, 	"sml_d" },
	  { 0b00000011, 	"sml_d" },
	  { 0b00000100, 	"sml_d" },
	  { 0b00000101, 	"sml_d" },
	  { 0b00000110, 	"eos" },
	  { 0b00000111, 	"lrg_d" },
	  { 0b00001000, 	"sml_d" },
	  { 0b00001001, 	"sml_d" },
	  { 0b00001010, 	"sml_d" },
	  { 0b00001011, 	"sml_d" },
	  { 0b00001100, 	"sml_d" },
	  { 0b00001101, 	"sml_d" },
	  { 0b00001110, 	"nop" },
	  { 0b00001111, 	"lrg_d" },
	  { 0b00010000, 	"sml_d" },
	  { 0b00010001, 	"sml_d" },
	  { 0b00010010, 	"sml_d" },
	  { 0b00010011, 	"sml_d" },
	  { 0b00010100, 	"sml_d" },
	  { 0b00010101, 	"sml_d" },
	  { 0b00010110, 	"nop" },
	  { 0b00010111, 	"lrg_d" },
	  { 0b00011000, 	"sml_d" },
	  { 0b00011001, 	"sml_d" },
	  { 0b00011010, 	"sml_d" },
	  { 0b00011011, 	"sml_d" },
	  { 0b00011100, 	"sml_d" },
	  { 0b00011101, 	"sml_d" },
	  { 0b00011110, 	"udef" },
	  { 0b00011111, 	"lrg_d" },
	  { 0b00100000, 	"sml_d" },
	  { 0b00100001, 	"sml_d" },
	  { 0b00100010, 	"sml_d" },
	  { 0b00100011, 	"sml_d" },
	  { 0b00100100, 	"sml_d" },
	  { 0b00100101, 	"sml_d" },
	  { 0b00100110, 	"udef" },
	  { 0b00100111, 	"lrg_d" },
	  { 0b00101000, 	"sml_d" },
	  { 0b00101001, 	"sml_d" },
	  { 0b00101010, 	"sml_d" },
	  { 0b00101011, 	"sml_d" },
	  { 0b00101100, 	"sml_d" },
	  { 0b00101101, 	"sml_d" },
	  { 0b00101110, 	"udef" },
	  { 0b00101111, 	"lrg_d" },
	  { 0b00110000, 	"sml_d" },
	  { 0b00110001, 	"sml_d" },
	  { 0b00110010, 	"sml_d" },
	  { 0b00110011, 	"sml_d" },
	  { 0b00110100, 	"sml_d" },
	  { 0b00110101, 	"sml_d" },
	  { 0b00110110, 	"udef" },
	  { 0b00110111, 	"lrg_d" },
	  { 0b00111000, 	"sml_d" },
	  { 0b00111001, 	"sml_d" },
	  { 0b00111010, 	"sml_d" },
	  { 0b00111011, 	"sml_d" },
	  { 0b00111100, 	"sml_d" },
	  { 0b00111101, 	"sml_d" },
	  { 0b00111110, 	"udef" },
	  { 0b00111111, 	"lrg_d" },
	  { 0b01000000, 	"sml_d" },
	  { 0b01000001, 	"sml_d" },
	  { 0b01000010, 	"sml_d" },
	  { 0b01000011, 	"sml_d" },
	  { 0b01000100, 	"sml_d" },
	  { 0b01000101, 	"sml_d" },
	  { 0b01000110, 	"pre_d" },
	  { 0b01000111, 	"lrg_d" },
	  { 0b01001000, 	"sml_d" },
	  { 0b01001001, 	"sml_d" },
	  { 0b01001010, 	"sml_d" },
	  { 0b01001011, 	"sml_d" },
	  { 0b01001100, 	"sml_d" },
	  { 0b01001101, 	"sml_d" },
	  { 0b01001110, 	"pre_d" },
	  { 0b01001111, 	"lrg_d" },
	  { 0b01010000, 	"sml_d" },
	  { 0b01010001, 	"sml_d" },
	  { 0b01010010, 	"sml_d" },
	  { 0b01010011, 	"sml_d" },
	  { 0b01010100, 	"sml_d" },
	  { 0b01010101, 	"sml_d" },
	  { 0b01010110, 	"pre_d" },
	  { 0b01010111, 	"lrg_d" },
	  { 0b01011000, 	"sml_d" },
	  { 0b01011001, 	"sml_d" },
	  { 0b01011010, 	"sml_d" },
	  { 0b01011011, 	"sml_d" },
	  { 0b01011100, 	"sml_d" },
	  { 0b01011101, 	"sml_d" },
	  { 0b01011110, 	"pre_d" },
	  { 0b01011111, 	"lrg_d" },
	  { 0b01100000, 	"sml_d" },
	  { 0b01100001, 	"sml_d" },
	  { 0b01100010, 	"sml_d" },
	  { 0b01100011, 	"sml_d" },
	  { 0b01100100, 	"sml_d" },
	  { 0b01100101, 	"sml_d" },
	  { 0b01100110, 	"pre_d" },
	  { 0b01100111, 	"lrg_d" },
	  { 0b01101000, 	"sml_d" },
	  { 0b01101001, 	"sml_d" },
	  { 0b01101010, 	"sml_d" },
	  { 0b01101011, 	"sml_d" },
	  { 0b01101100, 	"sml_d" },
	  { 0b01101101, 	"sml_d" },
	  { 0b01101110, 	"pre_d" },
	  { 0b01101111, 	"lrg_d" },
	  { 0b01110000, 	"udef" },
	  { 0b01110001, 	"udef" },
	  { 0b01110010, 	"udef" },
	  { 0b01110011, 	"udef" },
	  { 0b01110100, 	"udef" },
	  { 0b01110101, 	"udef" },
	  { 0b01110110, 	"udef" },
	  { 0b01110111, 	"udef" },
	  { 0b01111000, 	"udef" },
	  { 0b01111001, 	"udef" },
	  { 0b01111010, 	"udef" },
	  { 0b01111011, 	"udef" },
	  { 0b01111100, 	"udef" },
	  { 0b01111101, 	"udef" },
	  { 0b01111110, 	"udef" },
	  { 0b01111111, 	"udef" },
	  { 0b10000000, 	"sml_d" },
	  { 0b10000001, 	"sml_d" },
	  { 0b10000010, 	"sml_d" },
	  { 0b10000011, 	"sml_d" },
	  { 0b10000100, 	"sml_d" },
	  { 0b10000101, 	"sml_d" },
	  { 0b10000110, 	"pre_d" },
	  { 0b10000111, 	"lrg_d" },
	  { 0b10001000, 	"sml_d" },
	  { 0b10001001, 	"sml_d" },
	  { 0b10001010, 	"sml_d" },
	  { 0b10001011, 	"sml_d" },
	  { 0b10001100, 	"sml_d" },
	  { 0b10001101, 	"sml_d" },
	  { 0b10001110, 	"pre_d" },
	  { 0b10001111, 	"lrg_d" },
	  { 0b10010000, 	"sml_d" },
	  { 0b10010001, 	"sml_d" },
	  { 0b10010010, 	"sml_d" },
	  { 0b10010011, 	"sml_d" },
	  { 0b10010100, 	"sml_d" },
	  { 0b10010101, 	"sml_d" },
	  { 0b10010110, 	"pre_d" },
	  { 0b10010111, 	"lrg_d" },
	  { 0b10011000, 	"sml_d" },
	  { 0b10011001, 	"sml_d" },
	  { 0b10011010, 	"sml_d" },
	  { 0b10011011, 	"sml_d" },
	  { 0b10011100, 	"sml_d" },
	  { 0b10011101, 	"sml_d" },
	  { 0b10011110, 	"pre_d" },
	  { 0b10011111, 	"lrg_d" },
	  { 0b10100000, 	"med_d" },
	  { 0b10100001, 	"med_d" },
	  { 0b10100010, 	"med_d" },
	  { 0b10100011, 	"med_d" },
	  { 0b10100100, 	"med_d" },
	  { 0b10100101, 	"med_d" },
	  { 0b10100110, 	"med_d" },
	  { 0b10100111, 	"med_d" },
	  { 0b10101000, 	"med_d" },
	  { 0b10101001, 	"med_d" },
	  { 0b10101010, 	"med_d" },
	  { 0b10101011, 	"med_d" },
	  { 0b10101100, 	"med_d" },
	  { 0b10101101, 	"med_d" },
	  { 0b10101110, 	"med_d" },
	  { 0b10101111, 	"med_d" },
	  { 0b10110000, 	"med_d" },
	  { 0b10110001, 	"med_d" },
	  { 0b10110010, 	"med_d" },
	  { 0b10110011, 	"med_d" },
	  { 0b10110100, 	"med_d" },
	  { 0b10110101, 	"med_d" },
	  { 0b10110110, 	"med_d" },
	  { 0b10110111, 	"med_d" },
	  { 0b10111000, 	"med_d" },
	  { 0b10111001, 	"med_d" },
	  { 0b10111010, 	"med_d" },
	  { 0b10111011, 	"med_d" },
	  { 0b10111100, 	"med_d" },
	  { 0b10111101, 	"med_d" },
	  { 0b10111110, 	"med_d" },
	  { 0b10111111, 	"med_d" },
	  { 0b11000000, 	"sml_d" },
	  { 0b11000001, 	"sml_d" },
	  { 0b11000010, 	"sml_d" },
	  { 0b11000011, 	"sml_d" },
	  { 0b11000100, 	"sml_d" },
	  { 0b11000101, 	"sml_d" },
	  { 0b11000110, 	"pre_d" },
	  { 0b11000111, 	"lrg_d" },
	  { 0b11001000, 	"sml_d" },
	  { 0b11001001, 	"sml_d" },
	  { 0b11001010, 	"sml_d" },
	  { 0b11001011, 	"sml_d" },
	  { 0b11001100, 	"sml_d" },
	  { 0b11001101, 	"sml_d" },
	  { 0b11001110, 	"pre_d" },
	  { 0b11001111, 	"lrg_d" },
	  { 0b11010000, 	"udef" },
	  { 0b11010001, 	"udef" },
	  { 0b11010010, 	"udef" },
	  { 0b11010011, 	"udef" },
	  { 0b11010100, 	"udef" },
	  { 0b11010101, 	"udef" },
	  { 0b11010110, 	"udef" },
	  { 0b11010111, 	"udef" },
	  { 0b11011000, 	"udef" },
	  { 0b11011001, 	"udef" },
	  { 0b11011010, 	"udef" },
	  { 0b11011011, 	"udef" },
	  { 0b11011100, 	"udef" },
	  { 0b11011101, 	"udef" },
	  { 0b11011110, 	"udef" },
	  { 0b11011111, 	"udef" },
	  { 0b11100000, 	"lrg_l" },
	  { 0b11100001, 	"sml_l" },
	  { 0b11100010, 	"sml_l" },
	  { 0b11100011, 	"sml_l" },
	  { 0b11100100, 	"sml_l" },
	  { 0b11100101, 	"sml_l" },
	  { 0b11100110, 	"sml_l" },
	  { 0b11100111, 	"sml_l" },
	  { 0b11101000, 	"sml_l" },
	  { 0b11101001, 	"sml_l" },
	  { 0b11101010, 	"sml_l" },
	  { 0b11101011, 	"sml_l" },
	  { 0b11101100, 	"sml_l" },
	  { 0b11101101, 	"sml_l" },
	  { 0b11101110, 	"sml_l" },
	  { 0b11101111, 	"sml_l" },
	  { 0b11110000, 	"lrg_m" },
	  { 0b11110001, 	"sml_m" },
	  { 0b11110010, 	"sml_m" },
	  { 0b11110011, 	"sml_m" },
	  { 0b11110100, 	"sml_m" },
	  { 0b11110101, 	"sml_m" },
	  { 0b11110110, 	"sml_m" },
	  { 0b11110111, 	"sml_m" },
	  { 0b11111000, 	"sml_m" },
	  { 0b11111001, 	"sml_m" },
	  { 0b11111010, 	"sml_m" },
	  { 0b11111011, 	"sml_m" },
	  { 0b11111100, 	"sml_m" },
	  { 0b11111101, 	"sml_m" },
	  { 0b11111110, 	"sml_m" },
	  { 0b11111111, 	"sml_m" }
	};

	bool decode(std::istream& is) {

		bitreader br(is);
		uint8_t byte = 0;
		//header check
		magic_block.resize(4);
		raw_read(is, magic_block[0], 4);
		if (magic_block != "bvxn")
			error("magic is not bvxn");
		raw_read(is, out_byte);
		raw_read(is, block_size);
		
		//byte da legggere nel blocco da decodificare
		int32_t i = block_size;
		while (true) {
			byte = is.peek();
			auto found = (*h.find(byte));
			//chekko il byte e capisco l'opcode
			if (found.second == "med_d") {
				//medium distance("med_d")
				br(3);
				len = br(2);
				match_len = br(3);
				dis = br(6);
				match_len = (match_len << 2) | br(2) + 3;
				dis = dis | (br(8) << 6);
				while (len -- > 0) //from 1 to 16 bytes
				{
					out.insert(out.begin(), is.get());
				}
				int off = 0;
				for (int i = 0; i < match_len; i++) {
					out.insert(out.begin(), out[dis -1]);
					++off;
				}
			}
			else if (found.second == "lrg_d") {
				//"lrg_d" large distance
				len = br(2);
				match_len = br(3) + 3;
				br(3);
				//distance in LE
				raw_read(is, dis, 2);
				while (len-- > 0) //from 1 to 16 bytes
				{
					out.insert(out.begin(), is.get());
				}
				int off = 0;
				for (int i = 0; i < match_len; i++) {
					out.insert(out.begin(), out[dis -1]);
					++off;
				}
			}
			else if (found.second == "pre_d") {
				//previous distance ("pre_d")
				len = br(2);
				match_len = br(3) + 3;
				br(3);
				//use the last distance state
				while (len-- > 0) //from 1 to 16 bytes
				{
					out.insert(out.begin(), is.get());
				}
				int off = 0;
				for (int i = 0; i < match_len; i++) {
					out.insert(out.begin(), out[dis -1]);
					++off;
				}
			}
			else if (found.second == "lrg_m") { //se primi 4 but sono 1111
				//large match ("lrg_m")
				br(8);
				match_len = 16 + br(8);
				int off = 0;
				for (int i = 0; i < match_len; i++) {
					out.insert(out.begin(), out[dis -1]);
					++off;//serve perche dopo l'insert aumento la size di out di 1 che devo togliere per prendere il val 
				}
			}
			else if (found.second == "sml_m") {
				//small match ("sml_m")
				br(4);
				match_len = br(4);
				int off = 0;
				for (int i = 0; i < match_len; i++) {
					out.insert(out.begin(), out[dis -1]);
					++off;
				}
			}
			else if (found.second == "sml_l") {
				//small literal ("sml_l")
				br(4);
				len = br(4);
				while ((len)-- >0) //from 1 to 16 bytes
				{
					out.insert(out.begin(), is.get());
				}
			}
			else if (found.second == "lrg_l") {
				//large literal (lrg_l)
				br(8);
				len = 16 + br(8);
				while ((len)-- > 0) //from 1 to 16 bytes
				{
					out.insert(out.begin(), is.get());
				}
			}
			else if (found.first == 0b00001110)
			{
				br(8); //"nop"
			}
			else if (found.first == 0b00010110) {
				br(8); //"nop"
			}
			else if (found.first == 0b00000110) {
				//"eos"
				//da capire se devo 
				br(8);
				//br(64);//?not sure
				break;
			}
			else {
				//small distance ("sml_d)")
				len = br(2);
				match_len = br(3) + 3;
				dis = br(11);
				while (len-- > 0) //from 1 to 16 bytes
				{
					out.insert(out.begin(), is.get());
				}
				int off = 0;
				for (int i = 0; i < match_len; i++) {
					out.insert(out.begin(), out[dis -1]);
					++off;
				}
			}
			if (!is) {
				break;
			}
		}
		return true;
	}





};

int main(int argc, char** argv) {

	if (argc != 3) {
		error("syntax error:\n lzvn_decode <input file> <output file>\n");
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		error("impossible to read input file: argv[1]\n");
	}

	lzvn decoder;
	//decode
	decoder.decode(is);

	std::ofstream os(argv[2]);
	if (!os) {
		error("impossible to open the output file: argv[2]\n");
	}
	/*1)
	uint8_t sym;
	size_t size = decoder.out.size(); //perche senno facendo pop_back non scrivo tutti i 32 valori
	for (int a = 0; a < size; a++) {
		sym = decoder.out.back();
		decoder.out.pop_back();
		os << sym;
	}
	*/
	/*2)
	for (auto it = decoder.out.rbegin(); it != decoder.out.rend(); it++) {
		os << (*it);
	}
	*/
	//best way to write txt in out from a vector(from the end to the top)
	std::copy(decoder.out.rbegin(), decoder.out.rend(), std::ostream_iterator<uint8_t>(os));

	return EXIT_SUCCESS;
}

