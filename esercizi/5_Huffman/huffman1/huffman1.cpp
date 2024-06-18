#include <iostream> // EXIT
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>


//speaking about kind of pointer such as shared_ptr<int>
// std::unique_ptr <int> q (new int) -> compiler bloc
// auto x = q.get() -> x is a int*
// 

void syntax() {
	std::cout << "Syntax error: \n",
		"huffman1 [c|d] <input file> <output file>";
	exit(EXIT_FAILURE);
}

/*
Le stringhe letterali come:
"errore" sono di tipo const char[]
che può essere convertito in std::string temporaneo. 
Se la funzione error non accetta un const std::string&, 
ma solo un std::string&, 
non sarebbe possibile passare stringhe letterali direttamente alla funzione.
try to see what type is detected by auto if you assign "error"
	auto c = "error";
*/

void error(const std::string& s) {
	std::cout << s;
	exit(EXIT_FAILURE);
}

class bitreader
{
public:
	bitreader() {};

	~bitreader() {};
};

class bitwriter
{
public:

	bitwriter()	{}

	~bitwriter() {}

};

template <typename T, typename CT=uint64_t>
struct frequency {
	std::unordered_map <T, CT> counter_;
	void operator() (const T& val) {
		++counter_[val];
	}
	auto begin() {
		return counter_.begin;
	}
	auto end() {
		return counter_.end;
	}
	auto size() {
		return counter_.size();
	}
};

template <typename T>
struct node {
	T val_;
	uint64_t freq_;
	node* left_;
	node* right_;

	node(T& val, uint64_t freq) : val_(val), freq_(freq) {};


};

void compression(const std::string& fin,const std::string& fout) {
	std::ifstream is(fin, std::ios::binary); //it works also in txt mode
	if (!is) {
		error("impossible to open" + fin + "in read mode");
		exit(EXIT_FAILURE);
	}
	//vector con i numeri da contare
	std::vector<uint8_t> numbers{
		std::istream_iterator<uint8_t>(is),
		std::istream_iterator<uint8_t>()
	};

	//conto le frequenze dei numeri
	frequency <uint64_t> count;
	for (const auto& c : numbers) {
		count(c);
	}

	//create the huffman nodes



	std::ofstream os(fout, std::ios::binary);
	if (!os) {
		error("impossible to open" + fout + "in write mode");
		exit(EXIT_FAILURE);
	}
}


int main(int argc, char* argv[])
{
	std::string command = argv[1]; // c | d
	if (command == "c") {
		std::cout << "compression";
		compression(argv[2], argv[3]);
	}
	else if (command == "d") {
		std::cout << "decompression";
	}
	else syntax();

	
	
	return EXIT_SUCCESS;
}