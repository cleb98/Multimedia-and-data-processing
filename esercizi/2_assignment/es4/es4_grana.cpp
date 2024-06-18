#include <cstdint>
#include <vector>
#include <fstream>
#include <iterator>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

class bitwriter {
	uint8_t buffer_;
	int n_ = 0;
	std::ostream& os_;

	std::ostream& write_bit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++n_;
		if (n_ == 8) {
			raw_write(os_, buffer_);
			n_ = 0;
		}
		return os_;
	}

public:
	bitwriter(std::ostream& os) : os_(os) {}

	std::ostream& write(uint32_t u, uint8_t n) {
		//while (n --> 0) {
		//  write_bit(u >> n);
		//}
		for (int i = n - 1; i >= 0; --i) {
			write_bit(u >> i);
		}
		return os_;
	}

	std::ostream& operator()(uint32_t u, uint8_t n) {
		return write(u, n);
	}

	std::ostream& flush(uint32_t bit = 0) {
		while (n_ > 0) {
			write_bit(bit);
		}
		return os_;
	}

	~bitwriter() {
		flush();
	}
};

int main(int argc, char* argv[])
{
	if (argc != 3) {
		return 1;
	}

	std::ifstream is(argv[1]);
	if (!is) {
		return 1;
	}

	std::vector<int32_t> v{
		std::istream_iterator<int32_t>(is),
		std::istream_iterator<int32_t>()
	};

	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		return 1;
	}
	/*
	uint8_t buffer;
	int n = 0;
	for (const auto &x : v) {
		for (int i = 10; i >= 0; --i) {
			uint8_t cur_bit = (x >> i) & 1;
			buffer = (buffer << 1) | cur_bit;
			++n;
			if (n == 8) {
				raw_write(os, buffer);
				n = 0;
			}
		}
	}
	while (n > 0) {
		uint8_t cur_bit = 0;
		buffer = (buffer << 1) | cur_bit;
		++n;
		if (n == 8) {
			raw_write(os, buffer);
			n = 0;
		}
	}
	*/

	bitwriter bw(os);
	for (const auto& x : v) {
		bw(x, 11);
	}

	return 0;
}