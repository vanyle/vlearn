/*
 * FileReader.cpp
 *
 *  Created on: 17 avr. 2021
 *      Author: vanyle
 */

#include "file/FileReader.h"

#ifdef WIN32
#include <windows.h>
#include <codecvt>
#include <string>
#include <iomanip>
#endif

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace vio{

std::string getExecutablePath(){
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	return convert.to_bytes(path);
#else
	char result[ PATH_MAX ];
	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
	return std::string( result, (count > 0) ? count : 0 );
#endif
}
std::string getExecutableFolderPath(){
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	fs::path c(path);
	std::wstring folderpath = c.parent_path().wstring();
	return convert.to_bytes(folderpath);
#else
	char result[ PATH_MAX ];
	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
	const char *path;
	if (count != -1) {
	    path = dirname(result);
	    return std::string(path);
	}
	return "";
#endif
}

FileReader::FileReader(std::string filename) {
	// encode filename from utf8 to OS friendly format.
	// NB: yup, we have unicode support ! (for Windows)
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	fs::path p(convert.from_bytes(filename.c_str()));
	f = _wfopen((const wchar_t*)p.generic_u16string().c_str(),L"r");
#else
	f = fopen(filename.c_str(),"r");
#endif
}

int FileReader::nextChar(){
	return getc(f);
}
std::string FileReader::readLine(){
	std::string s;
	int c = getc(f);
	while((char)c != '\n' && c != EOF){
		s += (char)c;
		c = getc(f);
	}
	return s;
}
void FileReader::moveCursor(u32 i){
	fseek(f,(long)i,SEEK_SET);
}

FileReader::~FileReader() {
	fclose(f);
}

}
