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
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

template <typename T>
char* char_ptr(T& type) {
	return reinterpret_cast<char*>(&type);
}

template <typename T> 
void read_be(std::istream& is, T& bytes = 0, size_t size = sizeof(T)) {
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
			u = (u << 1) | readbit(); // |00000000 | 0000000 | 0000000 | 0000000 |
		}
		return u; 
	}



};

template <typename T>
struct qoa_samples
{
	std::vector<T> dx;
	std::vector<T> sx;

	void d_push(T& n) {
		dx.push_back(n);
	}
	void s_push(T& n) {
		sx.push_back(n);
	}
	auto raw_size() const{
		return (dx.size() + sx.size()) * sizeof(T);
	}
	auto size() const{
		return (dx.size() + sx.size());
	}

	auto d_begin() const{
		return dx.begin();
	}

	auto s_begin() const{
		return sx.begin();
	}

	auto d_end() const{
		return dx.end();
	}

	auto s_end() const{
		return sx.end();
	}
};

template <typename T>
void decomp_qoa(std::istream& is, qoa_samples<T>& q_samples) {
	/* qoa file = 8Byte file_header + num Frame at least 1 FRAME;
	FILE HEADER: char magic[4]; // magic bytes "qoaf"	uint32_t samples;
	// samples per channel in this file */
	bitreader br(is);

	std::string magic(4, ' ');

	is.read(reinterpret_cast<char*>(&magic[0]), 4);
	if (magic != "qoaf") {
		error("uncorrect magic word");
	}

	uint8_t s_byte;
	uint32_t samples;
	read_be(is, samples);
	int num_frame = static_cast<int>(ceil(samples / (256.0 * 20.0))); //solo se prox uint8 = 2

	//frame
	uint8_t num_channel = 0;
	uint32_t samplerate = 0;
	uint16_t fsamples, fsize;

	//for_each frame
	for (int i = 0; i < num_frame; i++) {
		raw_read(is, num_channel);
		read_be(is, samplerate, 3); //read 3 bytes, not sizeof(samplerate) that is 4!
		read_be(is, fsamples);
		read_be(is, fsize);

		using arr_16_4 = std::array<int16_t, 4>;
		std::vector<arr_16_4> history(num_channel); //sono due history per channel forse devo fa 4 var
		std::vector<arr_16_4> weights(num_channel);

		for (int a = 0; a < num_channel; a++) { 
			for (int b = 0; b < 4; b++)	{ //weight e history
				read_be(is, history[a][b], 2);
				read_be(is, weights[a][b], 2);
			}
		}
		//slice*channel iteration	 
		for (int j = 0; j < num_channel; j++) {
			//leggo 256 slice per channel
			for (int k = 0; k < 256; k++)
			{ 
				if (!is)   /* con questo controllo ottengo 20 smple in piu su un canale, ma non è possibile credo */
					break;

				std::array<uint32_t, 20> qr;
				//256 slice x channel da 64 bit
				uint8_t sf_quant = br(4);
				for (int s = 0; s < 20; s++) {
					qr[s] = br(3);
				}
				//dequantize scalefactor
				std::vector<double> dequant_tab = { 0.75, -0.75, 2.5, -2.5, 4.5, -4.5, 7, -7 };
				auto sf = round(pow(sf_quant + 1, 2.75));


				for (auto& it : qr) {
					auto res = sf * dequant_tab[it];
					if (res < 0) {
						res = ceil(res - 0.5);
					}
					else {
						res = floor(res + 0.5);
					}
					//j -> indice canale
					int p = 0;
					for (int n = 0; n < 4; n++) {
						p += history[j][n] * weights[j][n];
					}
					p >>= 13; //come dividere per 2**13
					int16_t sample = std::clamp(static_cast<int>(res) + p, -32768, 32767);
					if (k % 2 == 0)
					{
						q_samples.s_push(sample);
					}
					else
					{
						q_samples.d_push(sample);
					}
					int delta = static_cast<int>(res) >> 4;
					for (int n = 0; n < 4; n++) {
						if (history[j][n] < 0)
						{
							weights[j][n] += (-delta);
						}
						else
						{
							weights[j][n] += delta;
						}
					}
					for (int n = 0; n < 3; n++) {
						history[j][n] = history[j][n + 1];
					}
					history[j][3] = sample;
				}
			}
			if (!is) break;
			
		}

	}

}

template <typename T>
void save_wav(std::ostream& os,const qoa_samples<T>& q_samples) {
	os << "RIFF";
	int32_t file_size = q_samples.raw_size() + 44 - 8; //36 = 44 -8 Bytes
	raw_write(os, file_size);
	os << "WAVE";
	os << "fmt ";
	raw_write(os, 16);
	raw_write(os, 1, 2); //type format 
	raw_write(os, 2, 2); //nchannel
	raw_write(os, 44100); //sample rate
	raw_write(os, 176400); //(Sample Rate * BitsPerSample * Channels) / 8
	raw_write(os, 4, 2); //(BitsPerSample * Channels) / 8
	raw_write(os, 16, 2); // bit per sample
	os << "data";
	raw_write(os, q_samples.size(), 4);

	for (auto i = 0; i < q_samples.dx.size(); i++)
	{
		raw_write(os, q_samples.dx[i], 2);
		raw_write(os, q_samples.sx[i], 2);
	}
}

int main(int argc, char* argv[]) {
	
	if (argc != 3) {
		error("argc != 3");
	}
	/* -> mega errore, qoa_decomp è il nome del file
	std::string command = argv[1];
	if (command == "qoa_decomp") {
		std::cout << "decompressing qoa file";
	}
	*/ 
	std::string fin = argv[1];
	std::ifstream is(fin , std::ios::binary);
	if (!is) {
		error("cannot open" + fin);
	}
	//struct per storare i sample nel channel di destra/sx;
	qoa_samples<int16_t> q_samples;
	//decompress qoa
	decomp_qoa(is, q_samples);
	//save struct of sample dx e sx, in wav format
	std::ofstream os(argv[2], std::ios::binary);
	if (!os)
		error("cannot open output file");
	save_wav(os, q_samples);

	return EXIT_SUCCESS;
}
