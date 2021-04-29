/*
 * FileReader.h
 *
 *  Created on: 17 avr. 2021
 *      Author: vanyle
 */
#pragma once

#include <string>
#include "utils/utils.h"

namespace vio{

/**
Return the path to the executable containing the program (the .exe file)
The string is utf8 encoded.
This might for example return: C:\Users\Bob\Documents\program.exe
 */
std::string getExecutablePath();

/**
Return the path to the folder containing the current executable.
This might for example return: C:\Program Files\你好
*/
std::string getExecutableFolderPath();

/**
An object that represents a file.
Use int nextChar(); to read a character from a file
If nextChar() returns EOF = -1, the file was totally read. otherwise, you can cast this value to a char.
Use std::string readLine(); to read a line from the file.

The following program will display its own (binary) content (at least the 120 first characters)
@code
std::string p = getExecutablePath();
File file(p);
for(u32 i = 0;i < 120;i++){
	char c = file.nextChar();
	printf("%c",c);
}
@endcode
 */
class File {
	FILE * f = 0;
	std::string path;
	bool readMode = false;
	bool appendMode = false;

	void setReadMode();
	void setWriteMode();
	void setAppendMode();
public:

	bool isReadMode();
	bool isAppendMode();

	// filename should be encoded in utf8.
	File(std::string filename);
	File(File& f) = delete;
	File& operator=(const File& v) = delete;
	File& operator=(File&& v) = delete;

	~File();

	// management
	bool exists();
	void create();
	void remove();

	// reading
	void moveCursor(u32 i);
	std::string readLine();
	int nextChar();

	// read cannot read more than 4Go at once.
	// This does not matter as it's a bad idea anyway to allocate that much ram.
	std::string read(u32 bytes = -1);

	// writing
	void write(std::string s); // will remove the content of the file.
	void append(std::string s);

	u32 size();
};

}
