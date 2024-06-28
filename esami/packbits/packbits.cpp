#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <utility>
#include <unordered_map>

void error(const std::string& s) {
	std::cout << s;
	exit(EXIT_FAILURE);
}

void p_copy(std::string& s, int& counter, std::ostream& os) {
	os.put(counter - 1);
	for (char c : s)
	{
		os.put(c);
	}
	counter = 0; 
	s.clear();
}

void p_run(char sym, int& counter, std::ostream& os) {

	++counter; //perde sempre un carattere
	// srivo in output 257 - counter e il sym (2 byte)
	os.put(257 - counter);
	os.put(sym);
	counter = 0; //azzero il counter della run

}


//prende file input e scrive la codifica
void compression(const std::string& fin, const std::string& fout) {
	
	std::ifstream is(fin);
	if (!is) 
	{
		error("cannot open" + fin);
	}
	std::ofstream os(fout, std::ios::binary);
	if (!os)
	{
		error("cannot open" + fout);
	}

	std::string c_to_copy;
	int run_counter = 0;
	int copy_counter = 0;
	char c;
	char run_sym = ' ';
	
	while (true) {

		if (!is) {
			break;
		}

		c = is.get(); //a abb a r1 a; a b

		if (is.peek() == c) {

			if (run_counter == 127) {
				p_run(run_sym, run_counter, os);
			}
			else {
				run_counter++;
				run_sym = c;

				if (copy_counter > 0)
					p_copy(c_to_copy, copy_counter, os);
			}
		}
		else {
			if (run_counter > 0) {
				p_run(run_sym, run_counter, os);
			}	

			else
			{
				copy_counter++;
				c_to_copy += c;
				if(copy_counter == 128)
					p_copy(c_to_copy, copy_counter, os);
			}
		}



	}
	//eof
	os.put(128);
}

void decompression(const std::string& fin, const std::string& fout) {
	
	std::ifstream is(fin, std::ios::binary);
	if (!is)
	{
		error("cannot open" + fin);
	}

	std::ofstream os(fout);
	if (!os) {
		error("cannot open" + fin);
	}

	while (true)
	{
		uint8_t n = is.get();
		char c = ' ';
		int nbyte = 0;
		if (n > 128) {
			nbyte = 257 - n;
			c = is.get();
			for (int i = 0; i < nbyte; i++) {
				os.put(c);
			}
		} 
		else if (n < 128) {
			nbyte = n + 1; 
			for (int i = 0; i < nbyte; i++) {
				c = is.get();
				os.put(c);
			}
		}
		else // n = 128 -> EOF
		{
			break;
		}
		//else è 128 break loop 
	}		
}



int main(int argv, char* argc[]) {

	const std::string command = argc[1];
	if (command == "c") {
		compression(argc[2], argc[3]);
	}
	if (command == "d") {
		decompression(argc[2], argc[3]);
	}
	
	return EXIT_SUCCESS;
}