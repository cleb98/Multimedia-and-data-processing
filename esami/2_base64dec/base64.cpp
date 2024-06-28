#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <cstdint>

template <typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) 
{
    return os.write(reinterpret_cast<const char*>(&val), size);
}

class bitwiter 
{
    uint8_t buffer_;
    int nbits_ = 0;
    std::ostream& os_;

    std::ostream& write_bit(uint32_t bit) 
    {
        buffer_ = (buffer_ << 1) | (bit & 1);
        nbits_++;
        if (nbits_ == 8)
        {
            raw_write( os_, buffer_);
            nbits_ = 0;
        }
        return os_;
    }
public:
    bitwiter(std::ostream& os) : os_(os) {}
    //val da scrivere e con quanti bit
    std::ostream& write(uint32_t u, uint8_t nbits) 
    {
        while (nbits-- > 0)
        {
            write_bit(u >> nbits); //shifto dall'8° bit (piu signific) al bit meno significativo per prenderne uno alla volta e metterlo nel buffer
        }
        return os_;
    }
    std::ostream& flush(uint32_t bit = 0) 
    {
        while (nbits_ > 0) //se nbits_ > 0 significa che il buffer non è stato rimepito e quindi i bit nel buffer devono ancora essere scritti nell'ostream
        {
            write_bit(bit);
        }
        return os_;
    }

    ~bitwiter() 
    {
        flush();        
    }

};

std::string base64_decode(const std::string& input) {
    //costruire la tabella per associare ad ogni char di una stringa un val binario a 6bit
    /*building decoding table*/
    std::unordered_map<char, uint8_t> table; //l'uso delle unordered map è solo più efficiente potevo usare le mappe
    

    /*for (size_t i = 0; i <= 25; i++) {
        table['A' + i] = i;
    }
    for (size_t i = 26; i <= 51; i++) { // è sbagliato perche cosi farei a +26 al primo giro che nn è b!
        table['a' + i] = i;
    }
    for (size_t i = 52; i <= 61; i++) {
        table['0' + i] = i;
    }*/

    //from A to Z
    for (size_t i = 0; i <= 25; i++) {
        table['A' + i] = i;
    }
    //from a to z
    size_t offset = 26;
    for (size_t i = 0; i <= 25; i++) { 
        table['a' + i] = i + offset;
    }

    offset = 52;
    for (size_t i = 0; i <= 9; i++) {
        table['0' + i] = i + offset;
     }
    
    //+,/
    table['+'] = 62;
    table['/'] = 63;

    std::ostringstream ss;
    bitwiter bw(ss);
    for (char c : input) {
        if (c == '=') {
            break;
        }
        auto& it = table.find(c);
        if (it != table.end()) {
            bw.write(it.second, 6);
        }
        else
        {
            std::cerr << "the character" << std::hex << uint32_t(c) << "is not in the table";
        }
    }

    return ss.str();
}









int main() {
    //std::string input = "TWFu"; //man
    std::string output;
    std::string input = "TQ=="; 
    //std::string input = "TWE=";
    output = base64_decode(input);
    std::cout << output << "\n";
    
    return 0;
}
