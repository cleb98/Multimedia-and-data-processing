#include <cstdint>
#include <vector>
#include <fstream>
#include <iterator>

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	uint8_t buffer_;
	uint8_t n_ = 0;
	std::istream& is_;

public:
	bitreader(std::istream& is) : is_(is) {}

	uint32_t read_bit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

	uint32_t read(uint8_t n) {
		uint32_t u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return u;
	}

	uint32_t operator()(uint8_t n) {
		return read(n);
	}

	bool fail() const {
		return is_.fail();
	}

	explicit operator bool() const {
		return !fail();
	}
};

int main(int argc, char* argv[])
{
	if (argc != 3) {
		return 1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return 1;
	}

	bitreader br(is);

	std::vector<int32_t> v;
	while (1) {
		int32_t num = br(11);
		if (!br) {
			break;
		}
		if (num >= 1024) {
			num -= 2048;
		}
		v.push_back(num);
	}

	std::ofstream os(argv[2]);
	if (!os) {
		return 1;
	}

	copy(v.begin(), v.end(),
		std::ostream_iterator<int32_t>(os, "\n"));

	return 0;
}