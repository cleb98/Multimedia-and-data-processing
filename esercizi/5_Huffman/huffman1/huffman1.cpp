#include <iostream> // EXIT
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <utility> //std::pair


void syntax() {
	std::cout << "Syntax error: \n",
		"huffman1 [c|d] <input file> <output file>";
	exit(EXIT_FAILURE);
}

/*
Le stringhe letterali come:
"errore" sono di tipo const char[]*
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
	
	uint8_t nbits_ = 0;
	uint8_t buffer_ = 0;
	std::ostream& os_;

	//writebit
	void writebit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++ nbits_;
		if (nbits_ == 8) {
			os_.put(buffer_);
			nbits_ = 0; 
		}
	}


public:

	bitwriter(std::ostream& os): os_(os){};

	~bitwriter() {
		flush();
	}
	std::ostream& operator() (uint64_t val, uint64_t n_bit) {
		while (n_bit --> 0)	{
			//scrivo val con nbit 
			//dal bit piu sginificativo al meno
			writebit(val >> n_bit);
		}
	}
	std::ostream& flush(uint8_t pad = 0) {
		//write pad in buffer to fill it
	}
	
	
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

//template <typename T>
struct huffman {
	using pairtype = std::pair <uint64_t, uint64_t>;
	using maptype = std::unordered_map <uint8_t /*T*/, pairtype>;
	pairtype len_and_code;
	maptype codes_table; //{sym, (len, code)}

	void compute_codes(const node<uint8_t>* n, uint64_t len = 0, uint64_t code = 0) {
		//se è nodo non punta ad altri nodi è nodo foglia e quindi genero il codice
		if (n->left_ == nullptr) {
			//do smth with code_table
			codes_table[n->val_] = { len, code};
		}
		else{		// code = 0
			compute_codes(n->left_, len +1, (code << 1));  //code = 00
			compute_codes(n->right_, len + 1, (code << 1) | 1); //code = 01
		}
	}
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
		
		//first way to insert the new node in the right psotition with O(N) cost
		auto it = tree.begin();
		for (;it != tree.end(); ++it) { //it in questo caso punta a un puntatore
			if (new_node->freq_ >= (*it)->freq_)
				break;
		}
		tree.insert(it, new_node);
		//sotto spiego metodo piu veloce per insert in vector
		/* cancella solo queta riga
		//secodon way to insert new node using binary search algorithm -> lower_bound with lambda
		auto it = lower_bound(
			tree.begin(), 
			tree.end(), 
			new_node,
		[](const node<uint8_t>* a, const node<uint8_t>* b ){
			return a-> prob > b->prob;
			}
		);
		tree.insert(it, new node)
		/*
		Perché il Secondo Metodo è Più Efficiente
		Ricerca Binaria vs Ricerca Lineare:

		La ricerca binaria divide l'area di ricerca a metà ad ogni passo, riducendo il numero di confronti necessari per trovare la posizione di inserimento a O(log N), invece di confrontare sequenzialmente ogni elemento come nella ricerca lineare (O(N)).
		Questo è particolarmente vantaggioso quando il numero di elementi nel vettore (N) è grande, poiché O(log N) cresce molto più lentamente rispetto a O(N).
		Inserimento:

		Entrambi i metodi hanno un costo O(N) per l'inserimento effettivo a causa della necessità di spostare gli elementi per fare spazio al nuovo nodo.
		Tuttavia, il vantaggio del secondo metodo è nella velocità con cui trova la posizione di inserimento, migliorando così l'efficienza complessiva del processo.
			*/ 
		}
	// prendo il nodo root
	node<uint8_t>* root = tree.back(); // tree.front(); tree[0]; *tree.begin() 
	//creo huffman_table
	huffman table;
	table.compute_codes(root);


	std::ofstream os(fout, std::ios::binary);
	if (!os) {
		error("impossible to open" + fout + "in write mode");
		exit(EXIT_FAILURE);
	}
	//magic word | "HUFFMAN1" | 8byte
	os << "HUFFMAN1";
	
	//tableentries| uin8_t | Number of items in the following Huffman table
	uint8_t n_entries = static_cast<uint8_t> (table.codes_table.size());
	os << n_entries;

	//HuffmanTable| TableEntries triplets(sym = 8,bit, len = 5 bit,code = len)
	bitwriter bw(os);
	for (const auto& it : table.codes_table) {
		bw(it.first, 8);
		bw(it.second.first, 5);
		bw(it.second.second, it.second.first);

	}
	//NumSymbols| 32 bit unsigned integer stored in	big endian| Number of symbols encoded in the file.
	//Data| NumSymbols Huffman codes | Values encoded with Huffman codes, according to the previous	table


	

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

	
	_CrtDumpMemoryLeaks(); // Check for memory leaks
	return EXIT_SUCCESS;
}


