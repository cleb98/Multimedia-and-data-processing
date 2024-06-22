#include <iostream> // EXIT
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <utility> //std::pair
#include <numeric>
#include <ranges>

void syntax() {
	std::cout << "Syntax error: \n",
		"huffman1 [c|d] <input file> <output file>";
	exit(EXIT_FAILURE);
}

/*
Le stringhe letterali come:
"errore" sono di tipo const char[]*
che pu� essere convertito in std::string temporaneo. 
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

template <typename T>
std::istream& raw_read(std::istream& is, T& val) {
	return is.read(reinterpret_cast<char*> (&val), sizeof(T)); //POTREBBE DARE ERRORI LA SIZEOF(T) 
}
class bitreader
{
	uint8_t buffer_ = 0;
	uint8_t nb_ = 0;//num bits in the buffer
	std::istream& is_;

	uint8_t readbit() { // 
		// se nb_ = 0 fillo il buffer 
		// decremento nb_
		// shifto il buffer di nb_bit per ottenere il bit corrispondente ai |nb_| bit rimasti da leggere
		if (nb_ == 0)
		{
			raw_read(is_, buffer_); //buffer_ = is.get();
			nb_ = 8;
		}
		--nb_;
		return (buffer_ >> nb_) & 1;
	}



public:
	bitreader(std::istream& is): is_(is){};
	~bitreader() {};
	uint64_t operator() (size_t bit_to_r){ // bit_to_r -> amount of bit to read sequentially
		uint64_t val = 0;
		while (bit_to_r --> 0)
		{
			val = (val << 1) | readbit();
		}
		return val;
	}

};

template <typename T>
std::ostream& raw_write(std::ostream& os,const T& val) {
	return os.write(reinterpret_cast<const char*> (&val), sizeof(T));
}
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
			//scrivo val con nbit, dal bit piu sginificativo al meno
			writebit(val >> n_bit);
		}
		return os_;
	}
	std::ostream& flush(uint8_t pad = 0) {
		//write pad in buffer to fill it
		while (nbits_ > 0) // vedere quale altra condizione potrei metterci
		{
			writebit(pad);
		}
		return os_;
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
	
	double entropy() {
		double entropy = 0;
		//2 way to compute the total number or occurencies in the input file
		//1. accumulate + lambda
		auto tot = std::accumulate(counter_.begin(), counter_.end(), 0, [](int tot, std::pair<const T, CT>& p){ 
			return tot + p.second;
			} 
		);
		//2. iterator on unordered_map 
		//int tot = 0;
		//for (auto& c : counter_) {
		//	tot += int(c.second); 
		//}
		for (const auto& c : counter_) {
			double p = static_cast <double> (c.second) / tot;
			entropy -= p * std::log2(p);
		}
		return entropy;
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
		//se � nodo non punta ad altri nodi � nodo foglia e quindi genero il codice
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
	std::cout << "\n entropy " << count.entropy();

	//creo tutti i nodi e li ordino for huffman tree
	std::vector<node<uint8_t>*> tree; //voglio puntatori perch� � piu facile sortare i nodi come puntatori
	std::vector<std::unique_ptr<node<uint8_t>>> storage; 
	for (const auto& pair : count.counter_)
	{
		node<uint8_t>* n = new node<uint8_t>(pair.first, pair.second);
		storage.emplace_back(n);//dealloca i ptr creat con new quando saranno out of scope
		tree.push_back(n);

	}

	std::sort(tree.begin(), tree.end(), 
		[](const node<uint8_t>* a, const node<uint8_t>* b) 
			{return a->freq_ > b->freq_; 
		}
	);

	// aggrego toglo i due nodi in fondo e sommo le frequenze per ottenere quella del nodo figlio finche in tree c'� un solo nodo
	while (tree.size() > 1) {
		//aggregogli ultii due nodi e li tolgo dall'albero
		node<uint8_t>* a = tree.back();
		tree.pop_back();
		node<uint8_t>* b = tree.back();
		tree.pop_back();
		//creo new node dai due nodi aggregati
		node<uint8_t>* new_node = new node(a, b); 
		storage.emplace_back(new_node);
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
		Perch� il Secondo Metodo � Pi� Efficiente
		Ricerca Binaria vs Ricerca Lineare:

		La ricerca binaria divide l'area di ricerca a met� ad ogni passo, riducendo il numero di confronti necessari per trovare la posizione di inserimento a O(log N), invece di confrontare sequenzialmente ogni elemento come nella ricerca lineare (O(N)).
		Questo � particolarmente vantaggioso quando il numero di elementi nel vettore (N) � grande, poich� O(log N) cresce molto pi� lentamente rispetto a O(N).
		Inserimento:

		Entrambi i metodi hanno un costo O(N) per l'inserimento effettivo a causa della necessit� di spostare gli elementi per fare spazio al nuovo nodo.
		Tuttavia, il vantaggio del secondo metodo � nella velocit� con cui trova la posizione di inserimento, migliorando cos� l'efficienza complessiva del processo.
			*/ 
		}
	//nodo root
	//node<uint8_t>* root = tree.back(); // tree.front(); tree[0]; *tree.begin() 
	//creo huffman_table
	huffman table;
	table.compute_codes(tree.back()); //tree.back() � nodo root


	std::ofstream os(fout, std::ios::binary);
	if (!os) {
		error("impossible to open" + fout + "in write mode");
		exit(EXIT_FAILURE);
	}
	//magic word | "HUFFMAN1" | 8byte
	os << "HUFFMAN1";
	
	//tableentries| uin8_t | Number of items in the following Huffman table
	uint8_t n_entries = static_cast<uint8_t> (table.codes_table.size());
	//os << n_entries; //first way(not sure it works)
	os.put(n_entries); // way that works for sure
	//HuffmanTable| TableEntries triplets(sym = 8,bit, len = 5 bit,code = len)
	bitwriter bw(os);
	for (const auto& it : table.codes_table) {
		bw(it.first, 8);
		bw(it.second.first, 5);
		bw(it.second.second, it.second.first);

	}
	//NumSymbols| 32 bit unsigned integer stored in	big endian| Number of symbols encoded in the file.
	uint32_t numsym = numbers.size();
	bw(numsym, 32);
	//Data| NumSymbols Huffman codes | Values encoded with Huffman codes, according to the previous	table
	for (auto& c : numbers) {
		//scrivo il codice corrispondente a c
		auto len_code = table.codes_table[c]; //pair(len, code)
		bw(len_code.second, len_code.first ); //scrivo il code con lencode bit
	}
	//free the memory used by the pointer created with new: 
	 //we can use a unique_ptr as structur wherw will be added every new ptr and this stuctur will delete it for us
}

void decompression(const std::string& fin, const std::string& fout) {
	std::ifstream is(fin, std::ios::binary);
	if (!is) {	error("impossible to open" + fin + "in read mode");	}
	std::ofstream os(fout, std::ios::binary);
	if (!os) {	error("impossible to open" + fout + "in write mode");}
	//leggere con le condizioni, e mentre leggo un certo num di bit vado a scrivere testualmente con cout
	std::string magic(8, ' ');
	is.read(magic.data(), sizeof(magic.data()));
	if (magic != "HUFFMAN1") {
		error("decompression is failed");
	}
	bitreader br(is);
	//dcompress number of items in the following Huffman table (8bit)
	size_t table_size = br(8);
	if (table_size == 0) {
		table_size = 256;
	}
	// decompress a table in a unordered_map
	using triplet = std::pair<uint8_t, std::pair<uint64_t, uint64_t>>; //{ sym, { len, code } }
	std::vector<triplet> table;
	for (size_t i = 0; i < table_size; i++) {
		uint8_t sym = static_cast<uint8_t> (br(8));
		uint64_t len = br(5);
		uint64_t code = br(len);
		std::pair<uint64_t, uint64_t> len_code = { len , code };
		table.push_back({ sym, len_code });
	}
	
	size_t numsym = br(32);
	//sort della tabella per confrontare per primi i codici pi� corti(pi� probabili)
	std::sort(table.begin(), table.end(),
		[](const triplet& a,const triplet& b){
			//sym with same len are previously sorted alphabetically
			if (a.second.first == b.second.first) 
				return a.first < b.first;
			//sort for their len
			return a.second.first < b.second.first;
		}
	);
	
	//leggo i simboli encodati e li traduco
	for (size_t i = 0; i < numsym; i++)
	{
		uint64_t curcode = 0;
		uint64_t curlen = 0;
		//per ogni simbolo nel file prima della compressione
		size_t pos;
		for (pos = 0; pos < table.size(); pos++) { //table[pos] = {sym, {len, code}}
			auto sym = table[pos].first;
			auto len = table[pos].second.first;
			auto code = table[pos].second.second;
			//leggo len bit con br se curlen � 0, 
			// se devo confrontare il curcode con il code di un altro simbolo con stessa len dei bit gia letti,
			// non devo rileggere nulla con br
			auto bitread = len - curlen;
			curcode = (curcode << bitread) | br(bitread);
			// se devo leggere 3 bit ma ne o gia letto due devo andare avanti solo al prox giro di uno per ricalcolare curcode
			curlen = len; 
			if (curcode == code) {
				os.put(sym);
				break;
			}
		//should be a matches at least with last sym of of the table
		//pos may be at max (table.size()-1)
		if (pos == table.size()){
			error("no symbol to decode has been found");
			}

		}
	}
}

int main(int argc, char* argv[])
{
	{
	std::string command = argv[1]; // c | d
	if (command == "c") {
		std::cout << "compression";
		compression(argv[2], argv[3]);
	}
	else if (command == "d") {
		std::cout << "decompression";
		decompression(argv[2], argv[3]);
	}
	else syntax();


	}_CrtDumpMemoryLeaks(); // Check for memory leaks
	return EXIT_SUCCESS;
}


