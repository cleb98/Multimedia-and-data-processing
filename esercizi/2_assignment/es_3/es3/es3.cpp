#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>

void convert(const std::vector<int32_t> &v, std::ostream &os) {
	for (auto& i : v) {
		//Since the numbers are already in the correct format (32-bit 2's complement integers), the conversion process is straightforward.
		os << i << std::endl; 
	}
}

int main(int argc, char* argv[]) {

	//check number of arguments
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " <input file> " << argv[1] << " <output file> " << argv[2] << std::endl;
		return 1;
	}

	//open input file
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::cout << "Cannot open input file " << argv[1] << std::endl;
		return 2;
	}

	//open output file
	std::ofstream os(argv[2]);
	if (!os) {
		std::cout << "Cannot open output file " << argv[2] << std::endl;
		return 3;
	}

	//read input file that are 32 bit little endian integers of 2's complement format
	std::vector<int32_t> v;
	int32_t num;
	while (1) {
		is.read(reinterpret_cast<char*>(&num), sizeof(int32_t));
		if (!is) break;
		v.push_back(num);
	}
	// convert 32 little endian 2's complement in base 10 text format 
	convert(v, os); 




	return 0;
}