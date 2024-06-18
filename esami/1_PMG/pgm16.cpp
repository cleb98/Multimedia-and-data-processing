#include "mat.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

template <typename T>
std::istream& raw_read(std::istream& in, T& val) {
	in.read(reinterpret_cast<char*>(&val), sizeof(T));
	return in;
}

bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue)
{
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}
	std::string magicnumber;
	std::getline(is, magicnumber);
	if (magicnumber != "P5")
	{
		return false;
	}
	std::string comment;
	if (is.peek() == '#') {
		std::getline(is, comment);
	}
	else
	{
		return false;
	}

	int w, h;
	char newline;
	is >> w >> h;
	is.get(newline);
	if (newline != '\n') {
		return false;
	}
	else
	{
		img.resize(h, w);// @mem(&img.data[0], UINT8, 1, img.cols_, img.rows_, img.cols_)
	}
	// is salta gli spazi quindi vado a salvare in maxvalue come per i valori di h,w
	is >> maxvalue;
	newline = ' '; 
	is.get(newline);
	if (newline != '\n') {
		return false;
	}

	if (maxvalue < 256) { //funzia solo perche sono sicuro che se imm è a 1 B x pixel max_value = 255 se fosse minore scazza
		for (auto& pixel : img) {
			uint8_t b;
			raw_read(is, b); //leggo un byte per volta
			pixel = b; 
		}
	}
	else if (maxvalue > 255) {
	//read 2 bytes for time
		for (auto& pixel : img) {
			uint8_t b1;
			uint8_t b2;
			uint16_t b12 = 0;
			raw_read(is, b1);
			raw_read(is, b2);
			b12 = (b1 << 8) | b2;
			pixel = b12; 

		}
		// is.read(img.rawdata(), img.rawsize());// non si può usare in questo caso perchè non legge bene i valori; 

	}
	else {
		return false; 
	}

	return true;
}

//int main() {
//	std::string ia = "frog_bin.pgm";
//	std::string ib = "CR-MONO1-10-chest.pgm";
//	mat< uint16_t> img;
//	uint16_t maxvalue;
//	load(ib, img, maxvalue);
//	/*load(ib, img, maxvalue);*/
//	std::cout << img.rawsize(); 
//
//	return 0;
//
//}