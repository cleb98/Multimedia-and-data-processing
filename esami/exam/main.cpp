#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <array>
#include <cmath>

void error(const std::string& s) {
	std::cout << s;
	exit(EXIT_FAILURE);
}
template <typename T>
std::istream& raw_read(std::istream& is, T& val) {
	return is.read(reinterpret_cast<char*>(&val), sizeof(T));
}

template <typename T> 
void read_be(std::istream& is, T& bytes, size_t size = sizeof(T)) {
	bytes = 0;
	uint8_t byte;
	while (size --> 0)
	{
		bytes = (bytes << 8) | is.get();
	}
}

class bitreader
{
	uint8_t buffer_ = 0;
	uint8_t n_ = 0;
	std::istream& is_;

	uint64_t readbit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		-- n_;
		return (buffer_ >>  n_) & 1;
	}
public:
	bitreader(std::istream& is) : is_(is) {};
	~bitreader() {};

	uint64_t operator()(size_t nbits) {
		uint64_t u = 0;
		while (nbits-- > 0) {
			u = (u >> nbits) | readbit();
		}
		return u; 
	}



};


void read_qoaf() {

}
/*
struct {
	struct {
		char magic[4]; // magic bytes "qoaf"
		uint32_t samples; // samples per channel in this file
	} file_header;
	struct {
		struct {
			uint8_t num_channels; // no. of channels
			uint24_t samplerate; // samplerate in hz
			uint16_t fsamples; // samples per channel in this frame
			uint16_t fsize; // frame size (includes this header)
		} frame_header;
		struct {
			int16_t history[4]; // most recent last
			int16_t weights[4]; // most recent last
		} lms_state[num_channels];

		qoa_slice_t slices[256][num_channels];
	} frames[ceil(samples / (256 * 20))];

} qoa_file_t;
*/

int main(int argc, char* argv[]) {
	std::string command = argv[1];
	if (command == "qoa_decomp") {
		std::cout << "decompressing qoa file";
	}
	std::string fin = argv[2];
	std::ifstream is(argv[2], std::ios::binary);
	if (!is) {
		error("cannot open" + fin);
	}
	std::ofstream os(argv[3], std::ios::binary);
	if (!os)
		error("cannot open output file");
	/* qoa file = 8Byte file_header + num Frame at least 1 FRAME;
	FILE HEADER: char magic[4]; // magic bytes "qoaf"	uint32_t samples; 
	// samples per channel in this file */
	bitreader br(is);
	while (true)
	{
		if (!is)
			break;

		std::string magic;
		std::string comp = "qoaf";
		for (char c : comp)
		{
			raw_read(is, c);
			magic += c;
		}
		if (magic != comp)
			error("error in magic");
		uint8_t s_byte;
		uint32_t samples; 
		read_be(is, samples);
		int num_frame = ceil(samples / (256 * 20)); //solo se prox uint8 = 2
		//frame
		uint8_t num_channel;
		uint32_t samplerate;
		uint16_t fsamples, fsize;
		//for_each frame
		for (int i = 0; i < num_frame; i++) {
			raw_read(is, num_channel);
			read_be(is, samplerate, 3); //read 3 bytes, not sizeof(samplerate) that is 4!
			read_be(is, fsamples);
			read_be(is, fsamples);
			read_be(is, fsize);
			int dim = 4;
			using arr_16_4 = std::array<int16_t, 4>;
			std::array<arr_16_4, 2> hystory; //sono due history per channel forse devo fa 4 var
			std::array<arr_16_4, 2> weights;
			for (int a = 0; a < num_channel; a++) {
				//	//weight e history
				while (dim-- > 0)
				{
					read_be(is, hystory[a][dim], 2);

				}
				dim = 4;
				while (dim-- > 0)
				{
					read_be(is, weights[a][dim], 2);
				}
			}
			//slice*channel iteration
			uint8_t sf_quant;
			for (int j = 0; j < num_channel ; j++){
				for (int k = 0; k < 256; k++)
				{
					//256 slice x channel
					// leggo 4 bit del byte
					uint8_t sf_quant = br(4);


				}
			}

		}
	}		
	return EXIT_SUCCESS;
}
