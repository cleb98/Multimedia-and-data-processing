#include <iostream> // EXIT
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>


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
	node* left_ = nullptr;
	node* right_ = nullptr;

	node(T val, uint64_t freq) : val_(val), freq_(freq) {};
	//dai due nodi devo generarne uno nuovo (il nodo figlio)
	node(node* a, node* b): left_(a), right_(b), freq_(a-> freq_ + b-> freq_) {};

};

void compression(const std::string& fin,const std::string& fout) {
	std::ifstream is(fin, std::ios::binary); //it works also in txt mode
	if (!is) {
		error("impossible to open" + fin + "in read mode");
		exit(EXIT_FAILURE);
	}
	//vector con i numeri da contare
	std::vector<uint64_t> numbers{
		std::istream_iterator<uint8_t>(is),
		std::istream_iterator<uint8_t>()
	};

	//conto le frequenze dei numeri
	frequency <uint8_t> count;
	for (const auto& c : numbers) {
		count(c);
	}
	//creo tutti i nodi e li ordino for huffman tree
	std::vector<node<uint8_t>*> tree; //voglio puntatori perchè è piu facile sortare i nodi come puntatori
	for (const auto& pair : count.counter_)
	{
		tree.push_back(new node<uint8_t>(pair.first, pair.second));
	}

	std::sort(tree.begin(), tree.end(), 
		[](const node<uint8_t>* a, const node<uint8_t>* b) 
			{return a->freq_ > b->freq_; 
		}
	);

	// aggrego toglo i due nodi in fondo e sommo le frequenze per ottenere quella del nodo figlio finche in tree c'è un solo nodo
	while (tree.size() > 1) {
		//aggregogli ultii due nodi e li tolgo dall'albero
		node<uint8_t>* a = tree.back();
		tree.pop_back();
		node<uint8_t>* b = tree.back();
		tree.pop_back();
		//creo new node dai due nodi aggregati
		node<uint8_t>* new_node = new node(a, b); 
		//add new node nella posizione giusta
		
		auto it = tree.begin();
		for (;it != tree.end(); ++it) { //it in questo caso punta a un puntatore
			if (new_node->freq_ >= (*it)->freq_)
				break;
		}
		tree.insert(it, new_node);
			
		}
			
	
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