#include <cstdio>
#include <iostream>
#include <ostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>

// write the frquencies of a byte into an output file
void frequencies(std::vector<int> byte, std::ostream &output) {
	int count = 0;
	for (const auto& i : byte) {
		//std::cout << i << std::endl;
		//byte in output has to be written in two hex digits -> std::cout << std::setw(5) << std::setfill('0') << 42; will output "00042" because it pads with zeros instead of spaces.
		output << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << count << " occurs " << std::dec << i << std::endl;
		count++;
	}

}

int main(int argc, char* argv[]) {
	// command line arguments (take input file and produce output file) 
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << "input" << argv[1] << "output" << argv[2] << std::endl;
		return 1;
	}
	// check if input binary file can be opened
	std::ifstream input(argv[1], std::ios::binary); //std::ios::binary is a flag that tells the stream to open the file in binary mode
	if (!input) {
		std::cerr << "Error:canot open the input file " << argv[1] << std::endl;
		return 2;
	}
	// check if output file can be opened
	std::ofstream output(argv[2]); // no need to specify std::ios::binary because we are writing txt file as output
	if (!output) {
		std::cerr << "Error:canot open the output file " << argv[2] << std::endl;
		return 3;
	}
	// read input file
	std::vector<int> byteOccurences(256, 0); // vector of 256 elements because 1byte is represented by 8 bits, so 2^8 = 256, each element initialized to 0
	unsigned char byte = 0; // unsigned char is a type that can store 1 byte (8 bits) of data
	while (1) {
		input.read(reinterpret_cast<char*>(& byte), sizeof(byte)); // read 1 byte from input file and store it in byte, reinterpret_cast<char*>(&byte) is saying, "Consider the memory location where the variable byte is stored as if it's a sequence of char bytes." This is useful when you're dealing with raw binary data or when you want to work with the bytes that make up the binary representation of the variable.
		if(!input) break; // if input is finished, break the loop
		byteOccurences[byte]++; // increment the counter of the byte just read

	}
	// write output file
	frequencies(byteOccurences, output);

	return 0; // exit with no errors


}



