/*
 * FileReader.cpp
 *
 *  Created on: 17 avr. 2021
 *      Author: vanyle
 */

#include "file/File.h"

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

/*
FILE* getHandle(std::string filename,const char * mode){
#ifdef WIN32
	File * f;
	// convert utf8 to ansi aka utf8 -> utf16 (wstring) --reinterpret cast-> string
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	std::wstring wpath = convert.from_bytes(filename.c_str());
	errno_t ecode = _wfopen_s(&f,wpath.c_str(),L"rb");
	return f;
#elif
	return fopen(filename.c_str(),"rb");
#endif
}*/

File::File(std::string filename) {
	path = filename;
	readMode = false;
	f = 0;
	setReadMode();
}

bool File::exists(){
	return f != 0;
}
void File::create(){
	if(f == 0){
		readMode = false;
		setReadMode();
	}
}
bool File::isReadMode(){
	return readMode;
}
bool File::isAppendMode(){
	return appendMode;
}

u32 File::size(){
	fpos_t pos;
	fgetpos(f,&pos);
	fseek(f, 0L, SEEK_END);
	u32 s = ftell(f);
	fsetpos(f,&pos);
	return s;

}

void File::setWriteMode(){
	if(!readMode && !appendMode) return;
	if(f != 0) fclose(f);
	readMode = false;
	appendMode = false;
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	std::wstring wpath = convert.from_bytes(path.c_str());
	_wfopen_s(&f,wpath.c_str(),L"w+");
#elif
	f = fopen(path.c_str(),"w+");
#endif
}
void File::setReadMode(){
	if(readMode) return;
	if(f != 0) fclose(f);
	readMode = true;
	appendMode = false;
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	std::wstring wpath = convert.from_bytes(path.c_str());
	_wfopen_s(&f,wpath.c_str(),L"rb");
#elif
	f = fopen(path.c_str(),"rb");
#endif
}
void File::setAppendMode(){
	if(appendMode) return;
	if(f != 0) fclose(f);
	appendMode = true;
	readMode = false;
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	std::wstring wpath = convert.from_bytes(path.c_str());
	_wfopen_s(&f,wpath.c_str(),L"a+");
#elif
	f = fopen(path.c_str(),"a+");
#endif
}

void File::write(std::string s){
	setWriteMode();
	fwrite(s.c_str(),1,s.size(),f);
}
void File::append(std::string s){
	setAppendMode();
	fwrite(s.c_str(),1,s.size(),f);
}

void File::remove(){
	if(f != 0) fclose(f);
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	std::wstring wpath = convert.from_bytes(path.c_str());
	_wremove(wpath.c_str());
#elif
	remove(path.c_str());
#endif
}

int File::nextChar(){
	setReadMode();
	if(f == 0) return -1;
	return getc(f);
}
std::string File::read(u32 bytes){
	setReadMode();
	if(f == 0) return "";
	std::string s;
	if(bytes <= 32) s.reserve(bytes);

	int c = getc(f);
	while(c != EOF && bytes != 0){
		s += (char)c;
		bytes--;
	}
	return s;
}
std::string File::readLine(){
	setReadMode();
	if(f == 0) return "";
	std::string s;
	int c = getc(f);
	while((char)c != '\n' && c != EOF){
		s += (char)c;
		c = getc(f);
	}
	return s;
}
void File::moveCursor(u32 i){
	fseek(f,(long)i,SEEK_SET);
}

File::~File() {
	if(f == 0) return;
	fclose(f);
}

}
