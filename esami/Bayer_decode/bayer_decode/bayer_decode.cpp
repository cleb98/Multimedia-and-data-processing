#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <array>
#include "mat.h"

void error(const std::string& s) {
	std::cout << s;
	exit(EXIT_FAILURE);
}

bool save_pgm(const std::string& filename, mat<uint8_t>& img) {
	using namespace std;
	ofstream os(filename + ".pgm", ios::binary);
	if (!os) {
		error("cannot open output file");
	}
	//check how is done the png standard; what is the magic number
	os << "P5" << "\n";
	os << img.cols() << " " << img.rows() << "\n255\n"; //w, h of img in ASCII  cha
	os.write(img.rawdata(), img.rawsize());
	return 1;
}

using rgb = std::array<uint8_t, 3>;
bool pgm_to_ppm(const std::string& filename, mat<uint8_t>& img_pgm, mat<rgb>& img_rgb) {
	using namespace std;

	ofstream os(filename + ".ppm", ios::binary);
	if (!os) {
		error("cannot open output file");
	}
	os << "P6" << "\n";
	os << img_pgm.cols() << " " << img_pgm.rows() << "\n255\n"; //w, h of img in ASCII  cha
	//bayer pattern to build color img, even raw are rg rg rg rg, odd raw are gb gb gb gb
	img_rgb.resize(img_pgm.rows(), img_pgm.cols());
	//method 1: works woth assumption that img.cols, img.rows is 
	for (size_t r = 0; r < img_pgm.rows(); r += 2) {
		for (size_t c = 0; c < img_pgm.cols(); c += 2) {
			if (r + 1 < img_pgm.rows() && c + 1 < img_pgm.cols()) {
				img_rgb(r, c)[0] = img_pgm(r, c); //r
				img_rgb(r, c + 1)[1] = img_pgm(r, c + 1); //g
				img_rgb(r + 1, c)[1] = img_pgm(r + 1, c); //g
				img_rgb(r + 1, c + 1)[2] = img_pgm(r + 1, c + 1); //b
			}
		}
	}

	//metod 2: it works also with img with even dimensionality
	//for (size_t r = 0; r < img_pgm.rows(); r++) {
	//	for (size_t c = 0; c < img_pgm.cols(); c++) {
	//		//r, g even is red
	//		if ((r % 2 == 0) && (c % 2 == 0))
	//			img_rgb(r, c)[0] = img_pgm(r, c);
	//		// r,g odds is blue
	//		else if ((r % 2 != 0) && (c % 2 != 0))
	//			img_rgb(r, c)[2] = img_pgm(r, c);
	//		//in the rest of case are green
	//		else
	//		{
	//			img_rgb(r, c)[1] = img_pgm(r, c);
	//		}
	//	}
	//}

	if (img_rgb.data().empty()) {
		error("rgb image is empty");
	}
	os.write(img_rgb.rawdata(), img_rgb.rawsize());

	return 1;
}

//modify the rgb image in input conputing the green level of the whole img
bool interpolate_green(const std::string& filename, mat<rgb>& img_rgb) {
	
	using namespace std;
	ofstream os(filename + ".ppm", ios::binary);
	mat<rgb> img_g;

	int dh = 0;
	int dv = 0;
	int g_4, g_6, g_2, g_8, x_5, x_7, x_3, x_1, x_9;
	for (int r = 0; r < img_rgb.rows(); r++) {
		for (int c = 0; c < img_rgb.cols(); c++) {
			int i; 
			//se non entro nell'if non devo calcolare g5 (il pixel verde è gia presente nel immagine, nb. potrebbe  essere = 0)
			if (((r % 2 == 0) && (c % 2 == 0)) || ((r % 2 != 0) && (c % 2 != 0)))
			{
				//controllo se è r o b
				if ((r % 2 == 0) && (c % 2 == 0))
					i = 0; //se pari i è red
				else
					i = 0; // se dispari è blue
				//controllo non vada out of bound
				if ((c - 1) < 0)
					g_4 = 0;
				else
					g_4 = img_rgb(r, c - 1)[1];

				if ((c + 1) >= img_rgb.cols())
					g_6 = 0;
				else
					g_6 = img_rgb(r, c + 1)[1];


				if ((c + 2) >= img_rgb.cols())
					x_7 = 0;
				else
					x_7 = img_rgb(r, c + 2)[i];

				if ((c - 2) < 0)
					x_3 = 0;
				else
					x_3 = img_rgb(r, c - 2)[i];

				if ((r + 1) >= img_rgb.rows())
					g_8 = 0;
				else
					g_8 = img_rgb(r + 1, c)[1];

				if ((r + 2) >= img_rgb.rows())
					x_9 = 0;
				else
					x_9 = img_rgb(r + 2, c)[i];

				if ((r - 1) < 0)
					g_2 = 0;
				else
					g_2 = img_rgb(r - 1, c)[1];

				if ((r - 2) < 0)
					x_1 = 0;
				else
					x_1 = img_rgb(r - 2, c)[i];

				x_5 = img_rgb(r, c)[i];

				dh = abs(g_4 - g_6) + abs(x_5 - x_3 + x_5 - x_7);
				dv = abs(g_2 - g_8) + abs(x_5 - x_1 + x_5 - x_9);

				//compute g5
				int g_5 = 0;
				if (dh < dv)
				{
					g_5 = ((g_4 + g_6) / 2) + ((2 * x_5 - x_3 - x_7) / 4);
				}
				else if (dh > dv)
				{
					g_5 = ((g_2 + g_8) / 2) + ((2 * x_5 - x_1 - x_9) / 4);
				}
				else // dv = dh
				{
					g_5 = ((g_2 + g_4 + g_6 + g_8) / 4) + (((x_5 * 4) - x_1 - x_3 - x_9 - x_7) / 8);
				}
				//ora ho ogni pixel della nuova matrice e lo devo scrivere nella img di output
				img_rgb(r, c)[1] = g_5;

			}

		}
	}

	if (img_rgb.data().empty()) {
		error("rgb image is empty");
	}
	//inizio a scrivere il file
	os << "P6" << "\n";
	os << img_rgb.cols() << " " << img_rgb.rows() << "\n255\n"; //w, h of img in ASCII  cha

	os.write(img_rgb.rawdata(), img_rgb.rawsize());

	//check if img is empty before the return
	return 1;
}
	

 
bool load(const std::string &s,mat<uint8_t>& img , mat<uint16_t>& img16) {
	using namespace std;
	ifstream is(s, ios::binary);
	if (!is) {
		error("cannot open input file");
	}
	string magic;
	getline(is, magic);
	if (magic != "P5") {
		error("not magic number");
	}

	int w = 0, h = 0;
	is >> w;
	is >> h;
	img16.resize(h, w);
	uint16_t maxval = 0;
	is >> maxval;
	//newline
	//is.get();
	is.read(img16.rawdata(), img16.rawsize());

	img.resize(h, w);
	for (size_t i = 0; i < img.size(); i++) {
		img[i] = img16[i]/256;
	}

	if (img.data().empty())
		error("empty image");

	return 1;

}


int main() {
	mat<uint16_t> img16;
	mat<uint8_t> img;
	const std::string filenmae = "big.pgm";
	//faccio load di file name in img16 e 
	load(filenmae, img, img16);
	
	//salvo la img in uint8 come img_gray.pgm
	if (!save_pgm("img_gray", img)) {
		error("cannot save image in pgm format");
	}

	mat<rgb> img_rgb;
	pgm_to_ppm("img_color", img, img_rgb); //ricavo img_rgb da img usando il bayer pattern
	interpolate_green("img_green", img_rgb);//calcolo tutti i pixel del channel green per interpolazione
	//interpolate_red_blu pixels
	
	return 0;
}

