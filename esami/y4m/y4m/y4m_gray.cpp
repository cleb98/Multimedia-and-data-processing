#include "math.h"
#include "pgm.h"
#include "ppm.h"
#include "types.h"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <limits>


void error(const std::string& s) {
	std::cout << s;
	exit(EXIT_FAILURE);
}


//metodo write img
//metodo read/load img
template <typename T>
std::istream& raw_read(std::istream& is, T& val) {
	return is.read(reinterpret_cast<char*> (&val), sizeof(T));
}


bool y4m_extract_gray( const std::string& filename,	std::vector<mat<uint8_t>>& frames) 
{
	using namespace std;
	ifstream is(filename, ios::binary);
	//YUV4MPEG2 H288 W352 C420jpeg
	string magic;
	is >> magic; ////YUV4MPEG2
	if (magic != "YUV4MPEG2")
		error("the y4m file doesn't start with YUV4MPEG2");

	char c;
	int H, W; // height,width
	string C; //subsampling ratio
	char I; // i o p
	string F, A, X;
	while (is.peek() != '\n') {
		// devo vedere prox byte
		c = is.get();

		if (c == 'H')
			is >> H;//H288

		if (c == 'W')
			is >> W;//W352

		if (c == 'C')
			is >> C;

		if (c == 'I')
			is >> I;

		if (c == 'F')
			is >> F;

		if (c == 'A')
			is >> A;

		if (c == 'X')
			is >> X;

	}
	c = is.get(); //prende il \n

	

	while (true)
	{
		string header(5, ' ');
		//frame e header
		is.read(reinterpret_cast<char*> (&header[0]), 5);

		if (is.eof()) break;

		if (header != "FRAME") {
			error("no FRAME read");
		}
			
		//resto dell'header
		is.ignore(std::numeric_limits<streamsize>::max(), '\n');

		mat<uint8_t> img(H, W);
		is.read(img.rawdata(), img.rawsize());
		frames.push_back(img);
		is.ignore((W*H)/2);

	}
	return 1;

}



//int main(int argc, char* argv[]) {
//
//	mat<uint8_t> img;
//
//	std::vector<mat<uint8_t>> frames;
//	y4m_extract_gray("test1.y4m", frames);
//
//	return 0;
//}