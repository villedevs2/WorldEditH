#pragma once

#include <fstream>
using namespace std;

class BinaryFile
{
private:
	std::string filename;
	fstream stream;

	typedef union
	{
		unsigned int i;
		float f;
	} IntFloat;

public:
	typedef enum
	{
		MODE_READONLY,
		MODE_WRITEONLY,
		MODE_READWRITE,
	} BinaryFileMode;

	BinaryFile();
	~BinaryFile();

	bool open(std::string filename, BinaryFileMode mode);
	void close();
	unsigned char read_byte();
	unsigned short read_word();
	unsigned int read_dword();
	float read_float();
	void read_string(std::string *str);
	void read(char* buffer, int num_bytes);
	void write_byte(unsigned char byte);
	void write_word(unsigned short word);
	void write_dword(unsigned int dword);
	void write_float(float f);
	void write(char* buffer, int num_bytes);
	void write_string(std::string str);
};
