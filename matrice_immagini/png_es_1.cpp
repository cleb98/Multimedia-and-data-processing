
//Exercise 1
//Write a program that generates a gray level image of 256x256 pixels, in which the first row is made of 256 zeros,
//  the second one of 256 ones, the third one of 256 values 2 and so on.Save the image in the binary PGM and plain PGM formats,
//  described above.Verify that the image is viewable in XnView.
// The images should appear as a gradient from black to white from top to bottom.

//ex2 

//we need to model an image as a matrix.
//useful a template to manage both the types of images, grayscale and color.
//the template es a type T to represent the type of the pixel.

#include <cstdint>
#include "mat.h"
#include <string>
#include <fstream>
#include <iostream>
#include <iterator>//needed to use copy function




//write the image in memory as pgm file 
///third par used to specify if we want to save in binary mode(p5) or text_mode(p2) we use enum
enum class pgm_mode { plain = 2, binary = 5 };
bool write(const std::string& filename,const mat<uint8_t>& img, pgm_mode mode) { //mat non è const senno non mi fa usare il metodo rawdata
	//open the file
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		return false;
	}

	//check how is done the png standard; what is the magic number
	os << "P" << int(mode) << "\n";
	os << "# MDP 2023\n";//optional comment
	os << img.cols() << " " << img.rows() << "\n255\n"; //w, h of img in ASCII  chars base 10, max(img(r,c)) = 255
	//write dei pixel su os in base alla mode
	if (mode == pgm_mode::plain) {
		/*for (int r = 0; r < img.rows(); r++) {
			for (int c = 0; c < img.cols(); c++) {
				os << int(img(r, c)) << " ";}*/
	//insted of innested loop, it used only one
		/*for (int i = 0; i << img.size(); i++) {
			os << int(img[i]) << " ";
		}*/
	//using iterator
		//for (auto it = img.begin(); it != img.end(); it++) {
		//	os << int(*it) << " "; //os << it ->scrive l'iteratore stesso nel flusso di output, non il valore a cui punta,gli it rappresentano posizioni all'interno di una struttura dati, non i dati stessi. 
		//}
		//for (const auto& x : img) { // can i use because img has begin(), end() methods
		//	os << int(x) << ' ';
		//}
		copy(img.begin(), img.end(), std::ostream_iterator<int>(os, " ")); //last methods is copy algorithms
	}
	else {
		/* for (int r = 0; r < img.rows(); r++) {
			for (int c = 0; c < img.cols(); c++) {
				os << img(r, c);
			}
		} */
		//we can use the raw_data function the give me the data as bunch of bytes
		os.write(img.rawdata(), img.rawsize());
	}

	return true;
}

auto read(const std::string& filename) {
	mat<uint8_t> img;
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return img; //because means the img is empty
	}
	//need to se magic number
	std::string magic_number;
	std::getline(is, magic_number); //prende la prima linea dell'immagine, fino al primo \n ci darà quindi proprio il magic number
	
	pgm_mode mode;
	if (magic_number == "P2") {
		mode = pgm_mode::plain;
	}
	else if (magic_number == "P5")
	{
		mode = pgm_mode::binary;
	}
	else
	{
		return img;
	}
	if (is.peek() == '#') {
		std::string comment;
		std::getline(is, comment);
	}
	//read w, h, n°levels of pixels
	int width, height, nlevels; // are " " separed, so we can just use the stream on ifstream is, after nlevels we have \n so the flux will be stopped
	char newline;
	is >> width >> height >> nlevels; 
	is.get(newline); //extractor able to pick the next char after the nlevels, so the newline \n
	if (nlevels != 255 || newline != '\n')
	{
		return img;
	}

	img.resize(height, width);
	//implemtent the two way of reading file based on mode (plain->text or binary)
	if (mode == pgm_mode::plain) {
		for (auto& x : img) {
			int val;
			is >> val;
			x = val;
		}
	}
	else
	{
		is.read(img.rawdata(), img.rawsize());
	}

	return img;
}

int main() {
	{
		mat<uint8_t> img(256, 256);
		for (int r = 0; r < img.rows(); r++) {
			for (int c = 0; c < img.cols(); c++) {
				img(r, c) = r;
			}
		}
		write("test_plain.pgm", img, pgm_mode::plain);
		write("test_binary.pgm", img, pgm_mode::binary);

		img = read("frog_bin.pgm");

		if (img.empty()) {
			std::cout << "cannot see the img";
		}
	}
	_CrtDumpMemoryLeaks();
	return 0;
}
