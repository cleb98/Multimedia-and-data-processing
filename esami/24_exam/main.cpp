#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <cstdint>

void error(const std::string& s) {
	std::cout << "error,\n" << s;
}

template <typename T>
std::ostream& raw_write(std::ostream& os,const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<char*>(&val), size);
}

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	std::istream& is_;
	uint8_t buffer_ = 0;
	uint8_t n_ = 0;

	uint8_t readbit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::istream& is) : is_(is) {};
	~bitreader() {};

	uint64_t operator() (uint8_t nbit){
		uint64_t u = 0;
		while (nbit-- > 0) {
			u = (u << 1) | readbit();
		}
		return u;
	}
};



struct mobi_header
{
	/*
0	32	name	Database name (0 terminated)
32	2	attributes	(not important)
34	2	version	file version
36	4	creation date	Number of seconds since 01/01/1904
40	4	modification date	Number of seconds since 01/01/1904
44	4	last backup date	Number of seconds since 01/01/1904
48	4	modificationNumber	(not important)
52	4	appInfoID	(not important)
56	4	sortInfoID	(not important)
60	4	type	“BOOK”
64	4	creator	“MOBI”
68	4	uniqueIDseed	(not important)
72	4	nextRecordListID	(not important)
76	2	numberOfRecords	Number of records
	*/
	std::string name; //str a 32 char
	uint16_t attributes = 0, file_version = 0;

	uint32_t
		creation_date,
		modification_date,
		last_backup, //date
		modificationNumber,
		appInfoID,
		sortInfoID;

	std::string type;//“BOOK”
	std::string creator;//“MOBI"


	uint64_t not_important_byte;
	uint16_t num_records;
};

struct infoEntries
{
	uint32_t recordDataOffset;
	uint8_t recordAttributes;
	uint32_t uniqueID;//3 bytes

};

struct record {

	uint16_t Compressions;
	uint16_t unused;
	uint32_t TextLength, RecordCount;
	uint16_t RecordSize, EncryptionType, Unknown;
};

bool MOBIdecode(std::istream& is, std::ostream& os) {
	bitreader br(is);
	mobi_header h;
	h.name.resize(32);
	raw_read(is, h.name[0], 32);
	raw_read(is, h.attributes);
	raw_read(is, h.file_version);
	h.creation_date = br(32);
	h.modification_date = br(32);
	h.last_backup = br(32);
	//not important the next three fields
	raw_read(is, h.modificationNumber);
	raw_read(is, h.appInfoID);
	raw_read(is, h.sortInfoID);
	h.type.resize(4);
	h.creator.resize(4);
	raw_read(is, h.type[0], 4);
	raw_read(is, h.creator[0], 4);
	raw_read(is, h.not_important_byte, 8);
	h.num_records = br(16);

	os.put(0xEF);
	os.put(0xBB);
	os.put(0xBF);

	std::cout << "PDB name: " << h.name << "\n";
	std::cout << "Creation date (s): " << h.creation_date << "\n";
	std::cout << "Creator: " << h.creator << "\n";
	std::cout << "Records: " << h.num_records << "\n";
	//infoentries
	infoEntries e;
	std::vector <infoEntries> entries;
	for (size_t i = 0; i < h.num_records; i++) {
		e.recordDataOffset = br(32);
		e.recordAttributes = is.get();
		e.uniqueID = br(24);
		entries.push_back(e);
		std::cout << i << " - offset: " << e.recordDataOffset << " - id: " << e.uniqueID << "\n";
	}
	record rec;
	//move to the first record offset
	rec.Compressions = br(16);
	raw_read(is, rec.unused);
	rec.TextLength = br(32);
	rec.RecordCount = br(16);
	rec.EncryptionType = br(16);
	raw_read(is, rec.Unknown);
	//offset byte for 1st records
	is.seekg(entries[0].recordDataOffset);
	rec.Compressions = br(16);
	rec.unused = br(16);
	rec.TextLength = br(32);
	rec.RecordCount = br(16);
	rec.RecordSize = br(16);
	rec.EncryptionType = br(16);

	std::cout << "\nCompression: " << rec.Compressions << "\n";
	std::cout << "TextLength: " << rec.TextLength << "\n";
	std::cout << "RecordCount: " << rec.RecordCount << "\n";
	std::cout << "RecordSize: " << rec.RecordSize << "\n";
	std::cout << "EncryptionType: " << rec.EncryptionType << "\n";

	//move to offset byte for each records from the second until the last one

	std::vector <uint64_t> decoded;
	for (size_t i = 1; i < rec.RecordCount; i++) {
		int count = 0;
		while(count < 4096)
		{
			uint8_t byte_check = 0;
			uint64_t byte = 0;
			is.seekg(entries[i].recordDataOffset);
			byte_check = is.get();
			count += 1; //lwtto byte_chek
			if (byte_check == 1 || byte_check == 2 || byte_check == 3 || byte_check == 4 || byte_check == 5 || byte_check == 6 || byte_check == 7 || byte_check == 8) {
				raw_read(is, byte, byte_check);
				decoded.push_back(byte);
				count += byte_check; //leggo byte_check byte
			}
			else if (byte_check > 8 && byte_check < 0x80) {
				byte = is.get();
				decoded.push_back(byte);
				count += 2;

			}
			else if (byte_check > 0x7F && byte_check < 0xC0) //modified lz77
			{
				count += 2; //leggo 2 byte
				uint8_t fixed = br(2);
				if (fixed != 0b10) {
					error("first two bit are not 10");
				}
				uint16_t distance = br(11);
				if (distance < 1 || distance > 2047) {
					error("distance is out of range");
				}
				uint8_t len = br(3) + 3;
				if (len < 3 || len > 10)
					error("len is not in range 3 - 10");
				//vado indietro di distance position dal fondo del vector
				auto index = entries.size() - distance;
				for (size_t k = 0; k < len; k++) {
					decoded.push_back(decoded[index]);
				}
					//copio len byte da li
			}
			else if (byte_check > 0xC0 && byte_check <= 0xFF)
			{
				uint16_t space = 0x20;
				byte_check = (byte_check & 0b0111'1111);
				space = (space << 8) | byte_check;
				decoded.push_back(space);
			}

			//quando sono a 4096 byte smetto di leggere il record corrente

		}
		//scrivi in output il primo record?
		for (auto it : decoded) {
			os.put(it);
		}
	}
	

	
	return 1;
}

int main(int argc, char* argv[]) {

	if (argc != 3) {
		error("syntax-> MOBIdecode <input filename> <output filename>");
	}

	
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		error("impossible to open input file");
	}

	std::ofstream os(argv[2]);
	if (!os) {
		error("impossible to open output file");
	}

	MOBIdecode(is, os);

	return EXIT_SUCCESS;
}