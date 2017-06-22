#include "BinaryFile.h"

BinaryFile::BinaryFile()
{

}

BinaryFile::~BinaryFile(void)
{
}

bool BinaryFile::open(std::string filename, BinaryFileMode mode)
{
	ios_base::openmode m = 0;
	switch (mode)
	{
		case MODE_READONLY:		m = ios_base::in; break;
		case MODE_WRITEONLY:	m = ios_base::out; break;
		case MODE_READWRITE:	m = ios_base::in | ios_base::out; break;
	}

	stream.open(filename.c_str(), m | ios_base::binary);
	stream.seekg(0);

	return true;
}

void BinaryFile::close()
{
	stream.close();
}

void BinaryFile::read(char* buffer, int num_bytes)
{
	stream.read(buffer, num_bytes);
}

unsigned char BinaryFile::read_byte()
{
	return stream.get();
}

unsigned short BinaryFile::read_word()
{
	char t[2];
	stream.read(t, 2);

	return ((unsigned int)(t[0] & 0xff) << 8) |
		   (unsigned int)(t[1] & 0xff);
}

unsigned int BinaryFile::read_dword()
{
	char t[4];
	stream.read(t, 4);

	return ((unsigned int)(t[0] & 0xff) << 24) |
		   ((unsigned int)(t[1] & 0xff) << 16) |
		   ((unsigned int)(t[2] & 0xff) << 8) |
		   (unsigned int)(t[3] & 0xff);
}

float BinaryFile::read_float()
{
	char t[4];
	stream.read(t, 4);

	IntFloat f;

	f.i = ((unsigned int)(t[0] & 0xff) << 24) |
		  ((unsigned int)(t[1] & 0xff) << 16) |
		  ((unsigned int)(t[2] & 0xff) << 8) |
		  (unsigned int)(t[3] & 0xff);
	return f.f;
}

void BinaryFile::read_string(std::string *str)
{
	str->clear();

	int b;
	do
	{
		b = read_byte();
		str->push_back(b);
	} while (b != 0);
}

void BinaryFile::write(char* buffer, int num_bytes)
{
	stream.write(buffer, num_bytes);
}

void BinaryFile::write_byte(unsigned char byte)
{
	stream.put(byte);
}

void BinaryFile::write_word(unsigned short word)
{
	char t[2];
	t[0] = (word >> 8) & 0xff;
	t[1] = word & 0xff;
	stream.write(t, 2);
}

void BinaryFile::write_dword(unsigned int dword)
{
	char t[4];
	t[0] = (dword >> 24) & 0xff;
	t[1] = (dword >> 16) & 0xff;
	t[2] = (dword >> 8) & 0xff;
	t[3] = dword & 0xff;
	stream.write(t, 4);
}

void BinaryFile::write_float(float f)
{
	IntFloat ii;
	ii.f = f;

	char t[4];
	t[0] = (ii.i >> 24) & 0xff;
	t[1] = (ii.i >> 16) & 0xff;
	t[2] = (ii.i >> 8) & 0xff;
	t[3] = ii.i & 0xff;
	stream.write(t, 4);
}

void BinaryFile::write_string(std::string str)
{
	for (int j=0; j < str.length(); j++)
	{
		write_byte(str.at(j));
	}
	write_byte(0);	// null terminator
}