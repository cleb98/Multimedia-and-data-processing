#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <cstdint>
#include <vector>
#include <iterator>
#include <map>
#include <cassert>

using namespace std;

void error(const string& s) {
	cout << s;
	exit(EXIT_FAILURE);
}

template <typename T> 
istream& rawread(istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}


void FitCRC_Get16(uint16_t& crc, uint8_t byte)
{
	static const uint16_t crc_table[16] =
	{
		0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
		0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
	};
	uint16_t tmp;
	// compute checksum of lower four bits of byte
	tmp = crc_table[crc & 0xF];
	crc = (crc >> 4) & 0x0FFF;
	crc = crc ^ tmp ^ crc_table[byte & 0xF];
	// now compute checksum of upper four bits of byte
	tmp = crc_table[crc & 0xF];
	crc = (crc >> 4) & 0x0FFF;
	crc = crc ^ tmp ^ crc_table[(byte >> 4) & 0xF];

}

struct fit_header {

	uint8_t size_, protocol_version_;
	uint16_t profile_version_;
	int32_t data_size_;
	string data_type_;
	uint16_t crc_;
	
	bool read_(istream& is) {
		size_ = is.get();
		protocol_version_ = is.get();
		rawread(is, profile_version_);
		rawread(is, data_size_);
		data_type_.resize(4);
		rawread(is, data_type_[0], 4);
		if (data_type_ != ".FIT")
			error("error reading header");
		rawread(is, crc_);

		return true;
	}

	bool check_crc() {
		uint8_t byte = 0;
		uint16_t crc = 0;

		FitCRC_Get16(crc, size_);
		FitCRC_Get16(crc, protocol_version_);

		byte = profile_version_ & 0xff;
		FitCRC_Get16(crc, byte);
		byte = (profile_version_ >> 8) & 0xff;
		FitCRC_Get16(crc, byte);

		byte = data_size_ & 0xff;
		FitCRC_Get16(crc, byte);
		byte = (data_size_>>8) & 0xff;
		FitCRC_Get16(crc, byte);
		byte = (data_size_ >> 16) & 0xff;
		FitCRC_Get16(crc, byte);
		byte = (data_size_ >> 24) & 0xff;
		FitCRC_Get16(crc, byte);


		FitCRC_Get16(crc, data_type_[0]);
		FitCRC_Get16(crc, data_type_[1]);
		FitCRC_Get16(crc, data_type_[2]);
		FitCRC_Get16(crc, data_type_[3]);
		
		return (crc == crc_);
	}
};

using field = array<uint8_t, 3>;

struct record_message {
	uint8_t reserved_ = 0, architecture_ = 0;
	uint16_t global_message_;
	uint8_t numfield_ = 1;
	vector<field> field_def_;
	bool def_message_; // 1. def_msg, 0. data_msg

	void read_record(istream& is) {

		uint8_t byte = 0;
		byte = is.get();
		if ((byte >> 4) == 4)
			def_message_ = true;
		else
			def_message_ = false;
		uint8_t local_type = byte & 0x0f;

		if (def_message_) {
			rawread(is, reserved_);
			rawread(is, architecture_);
			rawread(is, global_message_);
			rawread(is, numfield_);

			field f;
			for (size_t i = 0; i < numfield_, i++) {
				rawread(is, f[0], 3);
				field_def_.push_back(f);
			}
		}


	}

};





int main(int argc, char* argv[]) {
	if (argc != 2) {
		error("syntax error: \n fitdump <input file .FIT> \n");
	}

	ifstream is(argv[1], ios::binary);
	if (!is) {
		error("ipossible to open input stream");
	}

	fit_header fit_h;
	fit_h.read_(is);
	//fit_h.FitCRC_Get16(fit_h.crc_, );
	if (fit_h.check_crc()) { //check_crc è 0 se è sbagliato
		cout << "Header CRC ok\n";
	}
	else 
		error("Header CRC is wrong");
	record_message record;

	vector <record_message> records;
	//perchè devo iterare su data_size_ che è la grandezza in byte dei records e non il num di records
	for (size_t i = 0; i < fit_h.data_size_; i++)
	{

	}
	//while (true) {
	//	record.read_record(is);

	//}


	return EXIT_SUCCESS;
}