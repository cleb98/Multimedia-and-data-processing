#include "math.h"
#include "pgm.h"
#include "ppm.h"
#include "types.h"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <limits>
#include <vector>

void error(const std::string& s) {
	std::cout << s;
	exit(EXIT_FAILURE);
}

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {
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


		mat<vec3b> img(H, W);
		//vector y 288*352
		vector<uint8_t> y(H*W);


		is.read(reinterpret_cast<char*>(&y[0]), y.size());
		//vector cb 288*352/4
		vector<uint8_t> cb((H * W) / 4);
		is.read(reinterpret_cast<char*>(&cb[0]), cb.size());
		//vector cr 288*352/4
		vector<uint8_t> cr((H * W) / 4);
		is.read(reinterpret_cast<char*>(&cr[0]), cr.size());

		//check se i vettori contengono i 
		/*
		keeping in mind that the values of Y must be between 16 and 235 and those of Cb and Cr must be between 16 and 240.
		If they are not, they must be saturated in the valid range (for example a value of Y equal to 7 must become a 16,
		a value of Cb equal to 245 must become 240). Also R, G and B must always be saturated between 0 and 255 before putting them in a byte.
		*/
		for (auto& it : y) {
			if (it < 16) {
				it = 16;
			}
			else if (it > 235) {
				it = 235;
			}
		}
		for (auto& it : cb) {
			if (it < 16) {
				it = 16;
			}
			else if (it > 240) {
				it = 240;
			}
		}
		for (auto& it : cr) {
			if (it < 16) {
				it = 16;
			}
			else if (it > 240) {
				it = 240;
			}
		}
		//rgb from ycbcr
		//prodotto matrice colonna
		vector<double> proj = { 1.164, 0.000, 1.596, 1.164, -0.392, -0.813, 1.164, 2.017, 0.000 };
		//frames.push_back(img);

		//per ogni pixel in y calcolo r, g, b
		int pos = 0;
		int poscbcr = 0;
		int a = 0;
		int b = 0;
		int c = 0;
		for (int x = 0; x < H; x++) {
			for (int y_ = 0; y_ < W; y_++)	{
				pos = x * W + y_;
				poscbcr = x / 2  * W/2 + y_ / 2; // W/2 è il numero di cols di cb, cr
				a = proj[0] * (y[pos]-16) + proj[1] * (cb[poscbcr] - 128) + proj[2] * (cr[poscbcr] - 128);
				b = proj[3] * (y[pos]-16) + proj[4] * (cb[poscbcr] - 128) + proj[5] * (cr[poscbcr] - 128);
				c = proj[6] * (y[pos]-16) + proj[7] * (cb[poscbcr] - 128) + proj[8] * (cr[poscbcr] - 128);

				img(x, y_)[0] = clamp(a, 0, 255);
				img(x, y_)[1] = clamp(b, 0, 255);
				img(x, y_)[2] = clamp(c, 0, 255);
			}
		}
		frames.push_back(img);

	}
	return 1;
}





int main(int argc, char* argv[]) {

	std::vector<mat<vec3b>> frames;
	y4m_extract_color("test1.y4m", frames);

	return 0;
}