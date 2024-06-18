#include <iostream>
#include <fstream>
#include <ostream>
#include <iterator>
#include <vector>
#include <algorithm>

// function to convert the input numbers in 32-bit binary little endian numbers in 2's complement
void convert(const std::vector<int32_t> &v, std::ostream& output) {
    for (const auto& i : v) {
        // write the number in binary little endian in the output file
        output.write(reinterpret_cast<const char*>(&i), sizeof(i));
        
    }
}

int main(int argc, char* argv[]) {

    // check number of arguments
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " input " << argv[1] << "output " << argv[2] << std::endl;
        return 1;
    }

    std::ifstream is(argv[1]);
    // open input file
    if (!is) {
        std::cerr << "Cannot open input file: " << argv[1] << std::endl;
        return 2;
    }

    //need output file with the same number of input file but in 2's complement
    std::ofstream os(argv[2], std::ios::binary);

    // open output file
    if (!os) {
        std::cerr << "Cannot open output file: " << argv[2] << std::endl;
        return 3;
    }


    // read input file int32 by int32
    std::vector <int32_t> v;
    int32_t num = 0;
    while (1) {
        is >> num;
        if (!is) break;
        v.push_back(num);
    }
    // use function to convert the input numbers in 32-bit binary little endian numbers in 2's complement
    convert(v, os);


    return 0;
}



